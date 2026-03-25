#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "detectors.h"
#include "cleaners.h"

void registerAdditionalRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/remove-state-suffixes").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    std::vector<std::vector<std::string>> result;
    for(const auto& row:parsed){
      std::vector<std::string> newRow;
      for(const auto& cell:row) newRow.push_back(removeStateSuffixes(cell));
      result.push_back(newRow);
    }
    crow::json::wvalue resp;
    resp["message"]="State suffixes removed";
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/remove-duplicate-words").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    std::vector<std::vector<std::string>> result;
    for(const auto& row:parsed){
      std::vector<std::string> newRow;
      for(const auto& cell:row) newRow.push_back(removeDuplicateWords(cell));
      result.push_back(newRow);
    }
    crow::json::wvalue resp;
    resp["message"]="Duplicate words removed";
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/detect-outliers").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto outliers=detectOutliers(parsed);
    crow::json::wvalue resp;
    resp["outlierCount"]=outliers.size();
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto missing=detectMissingValues(parsed);
    crow::json::wvalue result;
    result["missingCount"]=0;
    for(const auto& row:missing)
      for(bool m:row) if(m) result["missingCount"]++;
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
    result["message"]="Case standardized";
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
}

