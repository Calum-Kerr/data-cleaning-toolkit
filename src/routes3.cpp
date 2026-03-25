#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "cleaners.h"
#include "detectors.h"

void registerCleaningRoutes(crow::SimpleApp& app){
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
    result["message"]="Case standardized";
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
  CROW_ROUTE(app,"/api/fuzzy-deduplicate").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto deduped=fuzzyDeduplicateRows(parsed,0.85);
    crow::json::wvalue result;
    result["originalRows"]=parsed.size();
    result["deduplicatedRows"]=deduped.size();
    result["merged"]=parsed.size()-deduped.size();
    return crow::response(result);
  });
}

