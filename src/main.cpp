#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"
#include "audit.h"
#include "pdf_report.h"
#include <iostream>

void registerAdditionalRoutes(crow::SimpleApp& app);

int main(){
  crow::SimpleApp app;
  AuditLog auditLog;
  app.get_middleware<crow::CORSHandler>()
    .global()
    .headers("Content-Type","Accept")
    .methods("GET"_method,"POST"_method)
    .origin("*");
  CROW_ROUTE(app,"/api/health").methods("GET"_method)
  ([](){ crow::json::wvalue result; result["status"]="ok"; return crow::response(result); });
  CROW_ROUTE(app,"/api/parse").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    crow::json::wvalue result;
    result["rows"]=parsed.size();
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/clean").methods("POST"_method)
  ([&auditLog](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=removeDuplicates(parsed);
    crow::json::wvalue result;
    result["originalRows"]=parsed.size();
    result["cleanedRows"]=cleaned.size();
    result["message"]="Data cleaned successfully";
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto dups=detectDuplicates(parsed);
    crow::json::wvalue result;
    result["duplicateCount"]=0;
    for(bool d:dups) if(d) result["duplicateCount"]++;
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/remove-outliers").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=removeOutliers(parsed);
    crow::json::wvalue result;
    result["originalRows"]=parsed.size();
    result["cleanedRows"]=cleaned.size();
    result["outliersRemoved"]=parsed.size()-cleaned.size();
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/standardize-nulls").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=standardizeNullValuesInData(parsed);
    crow::json::wvalue result;
    result["message"]="Null values standardized";
    return crow::response(result);
  });
  registerAdditionalRoutes(app);
  app.port(8080).multithreaded().run();
}

