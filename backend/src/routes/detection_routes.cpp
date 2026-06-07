#include "crow_all.h"
#include "csv_parser.h"
#include "text_normalisation.h"
#include "string_issue_detectors.h"
#include "structural_cleaners.h"
#include "logger.h"
#include "rate_limiter.h"

std::string toCSV(const std::vector<std::vector<std::string>>& data){
  std::string result;
  for(const auto& row:data){
    for(size_t j=0;j<row.size();j++){
      if(j>0)result+=",";
      // CSV formula injection defence: prepend ' to cells starting with = + - @
      std::string safe=row[j];
      if(!safe.empty()){char first=safe[0];if(first=='='||first=='+'||first=='-'||first=='@')safe="'"+safe;}
      bool needsQuote=safe.find(',')!=std::string::npos||safe.find('"')!=std::string::npos||safe.find('\n')!=std::string::npos;
      if(needsQuote){
        result+="\"";
        for(char c:safe){
          if(c=='"')result+="\"\"";
          else result+=c;
        }
        result+="\"";
      }else result+=safe;
    }
    result+="\r\n";
  }
  return result;
}

void registerAdditionalRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/detect-outliers").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/detect-outliers", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto parsed=parseCSV(req.body);
    auto outliers=detectOutliers(parsed);
    crow::json::wvalue resp;
    resp["outlierCount"]=(int)outliers.size();
    logRequest("POST", "/api/detect-outliers", 200);
    return crow::response(resp);
  });
  CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/detect-missing", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto parsed=parseCSV(req.body);
    auto missing=detectMissingValues(parsed);
    int count=0;
    for(const auto& row:missing)
      for(bool m:row) if(m) count++;
    crow::json::wvalue result;
    result["missingCount"]=count;
    logRequest("POST", "/api/detect-missing", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/normalise-whitespace").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/normalise-whitespace", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto json=crow::json::load(req.body);
    std::string csvData=json["csvData"].s();
    auto parsed=parseCSV(csvData);
    auto normalized=trimWhitespace(parsed);
    crow::json::wvalue result;
    result["csvData"]=toCSV(normalized);
    result["message"]="Whitespace normalised";
    result["rows"]=(int)normalized.size();
    logRequest("POST", "/api/normalise-whitespace", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/standardise-case").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/standardise-case", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto json=crow::json::load(req.body);
    std::string csvData=json["csvData"].s();
    auto parsed=parseCSV(csvData);
    auto standardized=standardiseCase(parsed,"lower");
    crow::json::wvalue result;
    result["csvData"]=toCSV(standardized);
    result["message"]="Case standardised";
    logRequest("POST", "/api/standardise-case", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/remove-outliers").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/remove-outliers", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto parsed=parseCSV(req.body);
    auto cleaned=removeOutliers(parsed);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["cleanedRows"]=(int)cleaned.size();
    result["outliersRemoved"]=(int)(parsed.size()-cleaned.size());
    logRequest("POST", "/api/remove-outliers", 200);
    return crow::response(result);
  });
}

