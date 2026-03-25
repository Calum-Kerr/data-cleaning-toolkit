#include "crow_all.h"
#include "csv_parser.h"
#include "text_utils.h"
#include "cleaners.h"

void registerTextRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/remove-state-suffixes").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    std::vector<std::vector<std::string>> result;
    for(const auto& row:parsed){
      std::vector<std::string> newRow;
      for(const auto& cell:row)
        newRow.push_back(removeStateSuffixes(cell));
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
      for(const auto& cell:row)
        newRow.push_back(removeDuplicateWords(cell));
      result.push_back(newRow);
    }
    crow::json::wvalue resp;
    resp["message"]="Duplicate words removed";
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/natural-sort").methods("POST"_method)
  ([](const crow::request& req){
    auto parsed=parseCSV(req.body);
    auto sorted=naturalSort(parsed,0);
    crow::json::wvalue result;
    result["message"]="Data sorted naturally";
    result["rows"]=sorted.size();
    return crow::response(result);
  });
}

