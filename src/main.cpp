#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"
#include "audit.h"
#include "pdf_report.h"
#include <iostream>

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
  CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto missing=detectMissingValues(parsed);
    crow::json::wvalue result;
    result["missingCount"]=0;
    for(const auto& row:missing) for(bool m:row) if(m) result["missingCount"]++;
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/normalize-whitespace").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto normalized=trimWhitespace(parsed);
    crow::json::wvalue result;
    result["message"]="Whitespace normalized";
    result["rows"]=normalized.size();
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/standardize-case").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto standardized=standardizeCase(parsed,"lower");
    crow::json::wvalue result;
    result["message"]="Case standardized to lowercase";
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
  app.port(8080).multithreaded().run();
}

