#include "crow_all.h"
#include "csv_parser.h"
#include "text_normalisation.h"
#include "structural_cleaners.h"
#include "logger.h"
#include "rate_limiter.h"

void registerTextRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/remove-state-suffixes").methods("POST"_method)
  ([](const crow::request& req){
    const std::string clientIp = resolveClientIp(req.get_header_value("x-forwarded-for"), req.remote_ip_address);
    if (!checkRateLimit(clientIp)) {logRequest("POST", "/api/remove-state-suffixes", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
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
    logRequest("POST", "/api/remove-state-suffixes", 200);
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/remove-duplicate-words").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/remove-duplicate-words", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
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
    logRequest("POST", "/api/remove-duplicate-words", 200);
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/natural-sort/<int>").methods("POST"_method)
  ([](const crow::request& req, int colIndex){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/natural-sort", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto parsed=parseCSV(req.body);
    auto sorted=naturalSort(parsed,colIndex);
    crow::json::wvalue result;
    result["message"]="Data sorted naturally";
    result["rows"]=(int)sorted.size();
    logRequest("POST", "/api/natural-sort", 200);
    return crow::response(result);
  });
}

