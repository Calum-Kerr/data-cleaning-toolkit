#include "crow_all.h"
#include "csv_parser.h"
#include "csv_serializer.h"
#include "text_normalisation.h"
#include "string_issue_detectors.h"
#include "structural_cleaners.h"
#include "audit.h"
#include "logger.h"
#include "rate_limiter.h"
#include "alerts.h"
#include "analytics.h"
#include "documentation.h"
#include "backup.h"
#include "seo.h"
#include "load_test.h"
#include "database.h"
#include <iostream>
#include <filesystem>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <algorithm>

void registerAdditionalRoutes(crow::SimpleApp& app);
void registerTextRoutes(crow::SimpleApp& app);
void registerCleaningRoutes(crow::SimpleApp& app);
void registerFrontendRoutes(crow::SimpleApp& app);
void logFrontendDirStartup();

// Verify the ADMIN_API_KEY environment variable against the Authorization header.
// If the env var is not set or empty, fail closed: all admin routes are denied
// until the key is configured.
static bool checkAdminAuth(const crow::request& req) {
  const char* key = std::getenv("ADMIN_API_KEY");
  if (!key || key[0] == '\0') return false;  // not configured — deny
  std::string auth = req.get_header_value("authorization");
  std::string expected = std::string("Bearer ") + key;
  bool sameLength = auth.size() == expected.size();
  // Constant-time comparison: always scan the full expected length and
  // XOR-accumulate differences so timing does not reveal where a mismatch is.
  volatile unsigned char acc = 0;
  for (size_t i = 0; i < expected.size(); i++) {
    unsigned char a = i < auth.size() ? static_cast<unsigned char>(auth[i]) : 0;
    acc = acc | (a ^ static_cast<unsigned char>(expected[i]));
  }
  return sameLength && acc == 0;
}

// Escape a string for embedding inside a JSON string literal.  Control
// characters without a short escape are emitted as \u00xx.
static std::string jsonEscape(const std::string& s){
  std::string out;
  out.reserve(s.size()*2);
  for(char c:s){
    if(c=='\\')out+="\\\\";
    else if(c=='"')out+="\\\"";
    else if(c=='\n')out+="\\n";
    else if(c=='\r')out+="\\r";
    else if(c=='\t')out+="\\t";
    else if(c=='\b')out+="\\b";
    else if(c=='\f')out+="\\f";
    else if(static_cast<unsigned char>(c)<0x20){
      static const char* hex="0123456789abcdef";
      unsigned char u=static_cast<unsigned char>(c);
      out+="\\u00";
      out+=hex[(u>>4)&0xF];
      out+=hex[u&0xF];
    }
    else out+=c;
  }
  return out;
}

// Count cells whose value differs between an operation's input and output
// grids, compared element-wise over the overlapping rows and columns.
static int countChangedCells(const std::vector<std::vector<std::string>>& before,
                             const std::vector<std::vector<std::string>>& after){
  int changed=0;
  size_t rows=std::min(before.size(), after.size());
  for(size_t i=0;i<rows;i++){
    size_t cols=std::min(before[i].size(), after[i].size());
    for(size_t j=0;j<cols;j++)
      if(before[i][j]!=after[i][j]) changed++;
  }
  return changed;
}

int main(){
  // Privacy hardening: disable core dumps to prevent heap memory (which may
  // contain user-uploaded CSV data) from being written to disk on crash.
  if (prctl(PR_SET_DUMPABLE, 0) == -1) {
    std::cerr << "Warning: prctl(PR_SET_DUMPABLE,0) failed: " << std::strerror(errno) << std::endl;
  }
  // Privacy hardening: lock existing memory mappings to reduce the likelihood
  // of the kernel swapping heap pages (which may contain user-uploaded CSV
  // data) to a swap device under memory pressure.  MCL_FUTURE is deliberately
  // omitted — it would require every future mmap (including thread stacks) to
  // be locked, which quickly exhausts RLIMIT_MEMLOCK and causes std::thread
  // creation to throw std::system_error.  This is best-effort; if it fails the
  // application can still run — the warning is logged for ops visibility.
  if (mlockall(MCL_CURRENT) == -1) {
    std::cerr << "Warning: mlockall(MCL_CURRENT) failed: " << std::strerror(errno) << std::endl;
  }
  crow::SimpleApp app;
  writeStartupAlert();
  initializeDatabase();
  AuditLog auditLog;
  // Public health check — returns only status to avoid information disclosure.
  CROW_ROUTE(app,"/api/health").methods("GET"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/api/health", 429); return crow::response(429);}
    recordEndpointCall("/api/health");
    logRequest("GET", "/api/health", 200);
    crow::json::wvalue result; result["status"]="ok"; return crow::response(result);
  });
  // Public-facing health check — minimal to avoid leaking filesystem paths,
  // disk metrics, and backup age to unauthenticated callers.
  CROW_ROUTE(app,"/health").methods("GET"_method)
  ([](const crow::request&){
    crow::json::wvalue result;
    // Quick liveness check — just verify the server can respond.
    // Detailed diagnostics are behind /admin/health (requires ADMIN_API_KEY).
    result["status"]="ok";
    auto resp=crow::response(result);
    resp.set_header("Content-Type","application/json");
    return resp;
  });
  // Admin-only health check — exposes disk usage, backup age, and other
  // operational details.  Protected by the ADMIN_API_KEY environment variable.
  CROW_ROUTE(app,"/admin/health").methods("GET"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/admin/health", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) {logRequest("GET", "/admin/health", 401); return crow::response(401);}
    crow::json::wvalue result;

    std::error_code diskEc;
    auto space=std::filesystem::space("/tmp",diskEc);
    if(!diskEc){
      double freePct=(static_cast<double>(space.free)/space.capacity)*100.0;
      result["disk_free_percent"]=static_cast<int>(freePct);
      if(freePct<10.0){
        result["status"]="unhealthy";
        result["reason"]="disk space critical on /tmp";
        auto resp=crow::response(503,result);
        resp.set_header("Content-Type","application/json");
        return resp;
      }
    } else {
      result["disk_free_percent"]=-1;
      result["disk_error"]=diskEc.message();
    }

    std::error_code manifestEc;
    std::filesystem::path manifestPath("/tmp/toolkit_backups/manifest.txt");
    if(std::filesystem::exists(manifestPath,manifestEc)){
      auto mtime=std::filesystem::last_write_time(manifestPath,manifestEc);
      if(!manifestEc){
        auto now=std::filesystem::file_time_type::clock::now();
        auto age=std::chrono::duration_cast<std::chrono::hours>(now-mtime).count();
        result["last_backup_hours_ago"]=static_cast<int>(age);
        if(age>192){
          result["warning"]="last backup is over 8 days old";
        }
      }
    }

    result["status"]="ok";
    logRequest("GET", "/admin/health", 200);
    auto resp=crow::response(result);
    resp.set_header("Content-Type","application/json");
    return resp;
  });
  CROW_ROUTE(app,"/api/parse").methods("POST"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/parse", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    if (!tryAcquireConnection(clientIp)) return crow::response(429, "Too many concurrent requests from your IP");
    ConnectionGuard connGuard(clientIp);
    recordEndpointCall("/api/parse");
    auto parsed=parseCSV(req.body);
    crow::json::wvalue result;
    result["rows"]=static_cast<int>(parsed.size());
    logRequest("POST", "/api/parse", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/clean").methods("POST"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/clean", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    if (!tryAcquireConnection(clientIp)) return crow::response(429, "Too many concurrent requests from your IP");
    ConnectionGuard connGuard(clientIp);
    recordEndpointCall("/api/clean");
    AuditLog auditLog;
    auto json=crow::json::load(req.body);
    std::string csvData=json["csvData"].s();
    auto parsed=parseCSV(csvData);
    auto uppercased=standardiseCase(parsed,"upper");
    auditLog.addEntry("Uppercase All", countChangedCells(parsed,uppercased), static_cast<int>(parsed.size()), static_cast<int>(uppercased.size()));
    auto trimmed=trimWhitespace(uppercased);
    auditLog.addEntry("Trim Whitespace", countChangedCells(uppercased,trimmed), static_cast<int>(uppercased.size()), static_cast<int>(trimmed.size()));
    auto standardised=standardiseNullValuesInData(trimmed);
    auditLog.addEntry("Standardise Null Values", countChangedCells(trimmed,standardised), static_cast<int>(trimmed.size()), static_cast<int>(standardised.size()));
    auto cleaned=removeDuplicates(standardised);
    auditLog.addEntry("Remove Duplicates", 0, static_cast<int>(standardised.size()), static_cast<int>(cleaned.size()));
    std::string outputCsv = serializeToCSV(cleaned);
    std::string jsonBody="{\"cleanedData\":\"";
    jsonBody+=jsonEscape(outputCsv);
    jsonBody+="\"";
    jsonBody+=",\"originalRows\":"+std::to_string(parsed.size());
    jsonBody+=",\"cleanedRows\":"+std::to_string(cleaned.size());
    jsonBody+=",\"rowsRemoved\":"+std::to_string(parsed.size()-cleaned.size());
    jsonBody+=",\"message\":\"Data cleaned successfully\",\"auditLog\":[";
    for(size_t i=0;i<auditLog.entries.size();i++){
      if(i>0)jsonBody+=",";
      jsonBody+="{\"operationName\":\""+auditLog.entries[i].operationName+"\",\"cellsAffected\":"+std::to_string(auditLog.entries[i].cellsAffected)+",\"rowsBefore\":"+std::to_string(auditLog.entries[i].rowsBefore)+",\"rowsAfter\":"+std::to_string(auditLog.entries[i].rowsAfter)+",\"timestamp\":\""+auditLog.entries[i].timestamp.substr(0,19)+"\"}";
    }
    jsonBody+="]}";
    logRequest("POST", "/api/clean", 200);
    auto resp=crow::response(jsonBody);
    resp.set_header("Content-Type","application/json; charset=utf-8");
    return resp;
  });
  CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/detect-duplicates", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    if (!tryAcquireConnection(clientIp)) return crow::response(429, "Too many concurrent requests from your IP");
    ConnectionGuard connGuard(clientIp);
    recordEndpointCall("/api/detect-duplicates");
    auto parsed=parseCSV(req.body);
    auto dups=detectDuplicates(parsed);
    int count=0;
    for(bool d:dups) if(d) count++;
    crow::json::wvalue result;
    result["duplicateCount"]=count;
    logRequest("POST", "/api/detect-duplicates", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/analytics").methods("GET"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/api/analytics", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) {logRequest("GET", "/api/analytics", 401); return crow::response(401);}
    recordEndpointCall("/api/analytics");
    writeAnalyticsSummary();
    logRequest("GET", "/api/analytics", 200);
    crow::json::wvalue result; result["status"]="analytics logged"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/documentation").methods("GET"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/api/documentation", 429); return crow::response(429);}
    recordEndpointCall("/api/documentation");
    logRequest("GET", "/api/documentation", 200);
    return getApiDocumentation();
  });
  CROW_ROUTE(app,"/api/backup").methods("POST"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/backup", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) {logRequest("POST", "/api/backup", 401); return crow::response(401);}
    recordEndpointCall("/api/backup");
    createBackup();
    logRequest("POST", "/api/backup", 200);
    crow::json::wvalue result; result["status"]="backup created"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/seo").methods("GET"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/api/seo", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) {logRequest("GET", "/api/seo", 401); return crow::response(401);}
    recordEndpointCall("/api/seo");
    writeSeoReport();
    logRequest("GET", "/api/seo", 200);
    crow::json::wvalue result; result["status"]="seo report generated"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/load-test/<int>").methods("POST"_method)
  ([](const crow::request& req, int requestCount){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/load-test", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) {logRequest("POST", "/api/load-test", 401); return crow::response(401);}
    recordEndpointCall("/api/load-test");
    auto result=simulateLoad(requestCount);
    writeLoadTestReport(result);
    logRequest("POST", "/api/load-test", 200);
    crow::json::wvalue response; response["status"]="load test completed"; response["total"]=result.totalRequests; return crow::response(response);
  });
  CROW_ROUTE(app,"/api/requests/<int>").methods("GET"_method)
  ([](const crow::request& req, int limit){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("GET", "/api/requests", 429); return crow::response(429);}
    recordEndpointCall("/api/requests");
    crow::json::wvalue response; response["status"]="request history disabled"; logRequest("GET", "/api/requests", 200);
    return crow::response(response);
  });
  registerAdditionalRoutes(app);
  registerTextRoutes(app);
  registerCleaningRoutes(app);
  registerFrontendRoutes(app);
  logFrontendDirStartup();
  app.bindaddr("127.0.0.1").port(8080).concurrency(16).run();
}

