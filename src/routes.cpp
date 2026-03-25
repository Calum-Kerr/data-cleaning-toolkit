#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"

void registerAdditionalRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/detect-outliers").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto outliers=detectOutliers(parsed);
    crow::json::wvalue resp;
    resp["outlierCount"]=(int)outliers.size();
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto missing=detectMissingValues(parsed);
    int count=0;
    for(const auto& row:missing)
      for(bool m:row) if(m) count++;
    crow::json::wvalue result;
    result["missingCount"]=count;
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/normalize-whitespace").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto normalized=trimWhitespace(parsed);
    crow::json::wvalue result;
    result["message"]="Whitespace normalized";
    result["rows"]=(int)normalized.size();
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
  CROW_ROUTE(app,"/api/remove-outliers").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto cleaned=removeOutliers(parsed);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["cleanedRows"]=(int)cleaned.size();
    result["outliersRemoved"]=(int)(parsed.size()-cleaned.size());
    return crow::response(result);
  });
}

