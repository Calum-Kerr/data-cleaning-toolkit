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

void registerAdditionalRoutes(crow::SimpleApp& app);
void registerTextRoutes(crow::SimpleApp& app);
void registerCleaningRoutes(crow::SimpleApp& app);
void registerFrontendRoutes(crow::SimpleApp& app);
void logFrontendDirStartup();

int main(){
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
    recordEndpointCall("/api/backup");
    createBackup();
    logRequest("POST", "/api/backup", 200);
    crow::json::wvalue result; result["status"]="backup created"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/seo").methods("GET"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("GET", "/api/seo", 429); return crow::response(429);}
    recordEndpointCall("/api/seo");
    writeSeoReport();
    logRequest("GET", "/api/seo", 200);
    crow::json::wvalue result; result["status"]="seo report generated"; return crow::response(result);
  });
  CROW_ROUTE(app,"/api/load-test/<int>").methods("POST"_method)
  ([](const crow::request& req, int requestCount){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/load-test", 429); return crow::response(429);}
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
  app.port(8080).concurrency(16).run();
}

