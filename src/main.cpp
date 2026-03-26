#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"
#include "audit.h"
#include "pdf_report.h"
#include "logger.h"
#include "rate_limiter.h"
#include "alerts.h"
#include "analytics.h"
#include "documentation.h"
#include "backup.h"
#include "seo.h"
#include <iostream>

void registerAdditionalRoutes(crow::SimpleApp& app);
void registerTextRoutes(crow::SimpleApp& app);
void registerCleaningRoutes(crow::SimpleApp& app);
void registerFrontendRoutes(crow::SimpleApp& app);

int main(){
  crow::SimpleApp app;
  writeStartupAlert();
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
    recordEndpointCall("/api/parse");
    auto parsed=parseCSV(req.body);
    crow::json::wvalue result;
    result["rows"]=(int)parsed.size();
    logRequest("POST", "/api/parse", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/clean").methods("POST"_method)
  ([&auditLog](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/clean", 429); return crow::response(429);}
    recordEndpointCall("/api/clean");
    auto parsed=parseCSV(req.body);
    auto cleaned=removeDuplicates(parsed);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["cleanedRows"]=(int)cleaned.size();
    result["message"]="Data cleaned successfully";
    logRequest("POST", "/api/clean", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/detect-duplicates", 429); return crow::response(429);}
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
  registerAdditionalRoutes(app);
  registerTextRoutes(app);
  registerCleaningRoutes(app);
  registerFrontendRoutes(app);
  app.port(8080).concurrency(2).run();
}

