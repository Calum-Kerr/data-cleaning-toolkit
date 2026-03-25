#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"
#include "audit.h"
#include "pdf_report.h"
#include <iostream>

void registerAdditionalRoutes(crow::SimpleApp& app);
void registerTextRoutes(crow::SimpleApp& app);
void registerCleaningRoutes(crow::SimpleApp& app);
void registerFrontendRoutes(crow::SimpleApp& app);

int main(){
  crow::SimpleApp app;
  AuditLog auditLog;
  CROW_ROUTE(app,"/api/health").methods("GET"_method)
  ([](){ crow::json::wvalue result; result["status"]="ok"; return crow::response(result); });
  CROW_ROUTE(app,"/api/parse").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    crow::json::wvalue result;
    result["rows"]=(int)parsed.size();
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/clean").methods("POST"_method)
  ([&auditLog](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=removeDuplicates(parsed);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["cleanedRows"]=(int)cleaned.size();
    result["message"]="Data cleaned successfully";
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto dups=detectDuplicates(parsed);
    int count=0;
    for(bool d:dups) if(d) count++;
    crow::json::wvalue result;
    result["duplicateCount"]=count;
    return crow::response(result);
  });
  registerAdditionalRoutes(app);
  registerTextRoutes(app);
  registerCleaningRoutes(app);
  registerFrontendRoutes(app);
  app.port(8080).multithreaded().run();
}

