#include "crow_all.h"
#include "csv_parser.h"
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

void registerAdditionalRoutes(crow::SimpleApp& app);
void registerTextRoutes(crow::SimpleApp& app);
void registerCleaningRoutes(crow::SimpleApp& app);
void registerFrontendRoutes(crow::SimpleApp& app);
void logFrontendDirStartup();

// Verify the ADMIN_API_KEY environment variable against the Authorization header.
// If the env var is not set, authentication is skipped for backward compatibility.
static bool checkAdminAuth(const crow::request& req) {
  const char* key = std::getenv("ADMIN_API_KEY");
  if (!key || key[0] == '\0') return true;  // not configured — allow
  std::string auth = req.get_header_value("authorization");
  if (auth.empty()) return false;
  std::string expected = std::string("Bearer ") + key;
  return auth == expected;
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
  CROW_ROUTE(app,"/api/health").methods("GET"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/health", 429); return crow::response(429);}
    recordEndpointCall("/api/health");
    logRequest("GET", "/api/health", 200);
    crow::json::wvalue result; result["status"]="ok"; return crow::response(result);
  });
  CROW_ROUTE(app,"/health").methods("GET"_method)
  ([](const crow::request&){
    crow::json::wvalue result;

    // check disk space on /tmp
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

    // check last backup age from manifest file mtime
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
    auto resp=crow::response(result);
    resp.set_header("Content-Type","application/json");
    return resp;
  });
  CROW_ROUTE(app,"/api/parse").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/parse", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    recordEndpointCall("/api/parse");
    auto parsed=parseCSV(req.body);
    crow::json::wvalue result;
    result["rows"]=static_cast<int>(parsed.size());
    logRequest("POST", "/api/parse", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/clean").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/clean", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    recordEndpointCall("/api/clean");
    AuditLog auditLog;
    auto json=crow::json::load(req.body);
    std::string csvData=json["csvData"].s();
    auto parsed=parseCSV(csvData);
    int rowsBefore=static_cast<int>(parsed.size());
    auditLog.addEntry("Uppercase All", 0, rowsBefore, rowsBefore);
    auto uppercased=standardiseCase(parsed,"upper");
    int rowsAfterUpper=static_cast<int>(uppercased.size());
    auditLog.addEntry("Trim Whitespace", 0, rowsAfterUpper, rowsAfterUpper);
    auto trimmed=trimWhitespace(uppercased);
    int rowsAfterTrim=static_cast<int>(trimmed.size());
    auditLog.addEntry("Standardise Null Values", 0, rowsAfterTrim, rowsAfterTrim);
    auto standardised=standardiseNullValuesInData(trimmed);
    int rowsAfterStandard=static_cast<int>(standardised.size());
    auditLog.addEntry("Remove Duplicates", 0, rowsAfterStandard, rowsAfterStandard);
    auto cleaned=removeDuplicates(standardised);
    std::string outputCsv;
    outputCsv.reserve(req.body.size());
    for(const auto& row:cleaned){
      for(size_t j=0;j<row.size();j++){
        if(j>0)outputCsv+=",";
        const auto& cell=row[j];
        bool needsQuote=cell.find(',')!=std::string::npos||cell.find('"')!=std::string::npos||cell.find('\n')!=std::string::npos;
        if(needsQuote){
          outputCsv+="\"";
          for(char c:cell){
            if(c=='"')outputCsv+="\"\"";
            else outputCsv+=c;
          }
          outputCsv+="\"";
        }else outputCsv+=cell;
      }
      outputCsv+="\r\n";
    }
    std::string jsonBody="{\"cleanedData\":\"";
    jsonBody.reserve(outputCsv.size()*2);
    for(char c:outputCsv){
      if(c=='\\')jsonBody+="\\\\";
      else if(c=='"')jsonBody+="\\\"";
      else if(c=='\n')jsonBody+="\\n";
      else if(c=='\r')jsonBody+="\\r";
      else jsonBody+=c;
    }
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
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/detect-duplicates", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
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
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/analytics", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) return crow::response(401);
    recordEndpointCall("/api/analytics");
    writeAnalyticsSummary();
    logRequest("GET", "/api/analytics", 200);
    crow::json::wvalue result; result["status"]="analytics logged"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/documentation").methods("GET"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/documentation", 429); return crow::response(429);}
    recordEndpointCall("/api/documentation");
    logRequest("GET", "/api/documentation", 200);
    return getApiDocumentation();
  });
  CROW_ROUTE(app,"/api/backup").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/backup", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) return crow::response(401);
    recordEndpointCall("/api/backup");
    createBackup();
    logRequest("POST", "/api/backup", 200);
    crow::json::wvalue result; result["status"]="backup created"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/seo").methods("GET"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/seo", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) return crow::response(401);
    recordEndpointCall("/api/seo");
    writeSeoReport();
    logRequest("GET", "/api/seo", 200);
    crow::json::wvalue result; result["status"]="seo report generated"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/load-test/<int>").methods("POST"_method)
  ([](const crow::request& req, int requestCount){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/load-test", 429); return crow::response(429);}
    if (!checkAdminAuth(req)) return crow::response(401);
    recordEndpointCall("/api/load-test");
    auto result=simulateLoad(requestCount);
    writeLoadTestReport(result);
    logRequest("POST", "/api/load-test", 200);
    crow::json::wvalue response; response["status"]="load test completed"; response["total"]=result.totalRequests; return crow::response(response);
  });
  CROW_ROUTE(app,"/api/requests/<int>").methods("GET"_method)
  ([](const crow::request& req, int limit){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/requests", 429); return crow::response(429);}
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

