#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "cleaners.h"
#include "detectors.h"
#include "logger.h"
#include "rate_limiter.h"

void registerCleaningRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/standardize-nulls").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=standardizeNullValuesInData(parsed);
    crow::json::wvalue result;
    result["message"]="Null values standardized";
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/fuzzy-deduplicate/<double>").methods("POST"_method)
  ([](const crow::request& req, double threshold){
    auto parsed=parseCSV(req.body);
    auto deduped=fuzzyDeduplicateRows(parsed,threshold);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["deduplicatedRows"]=(int)deduped.size();
    result["merged"]=(int)(parsed.size()-deduped.size());
    return crow::response(result);
  });
}

