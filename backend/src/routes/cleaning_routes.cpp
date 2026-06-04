#include "crow_all.h"
#include "csv_parser.h"
#include "text_normalisation.h"
#include "structural_cleaners.h"
#include "string_issue_detectors.h"
#include "logger.h"
#include "rate_limiter.h"
#include "find_replace_rules.h"
#include "cluster_detection.h"
#include "csv_serializer.h"

void registerCleaningRoutes(crow::SimpleApp& app){
  CROW_ROUTE(app,"/api/standardise-nulls").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/standardise-nulls", 429); return crow::response(429);}
    if (req.body.size() > 50 * 1024 * 1024) return crow::response(413, "Payload too large. Maximum 50MB.");
    auto parsed=parseCSV(req.body);
    auto cleaned=standardiseNullValuesInData(parsed);
    crow::json::wvalue result;
    result["message"]="Null values standardised";
    logRequest("POST", "/api/standardise-nulls", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/fuzzy-deduplicate/<double>").methods("POST"_method)
  ([](const crow::request& req, double threshold){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/fuzzy-deduplicate", 429); return crow::response(429);}
    auto parsed=parseCSV(req.body);
    auto deduped=fuzzyDeduplicateRows(parsed,threshold);
    crow::json::wvalue result;
    result["originalRows"]=(int)parsed.size();
    result["deduplicatedRows"]=(int)deduped.size();
    result["merged"]=(int)(parsed.size()-deduped.size());
    logRequest("POST", "/api/fuzzy-deduplicate", 200);
    return crow::response(result);
  });
  CROW_ROUTE(app,"/api/find-replace").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/find-replace", 429); return crow::response(429);}
    try {
      auto json=crow::json::load(req.body);
      std::string csvData=json["csvData"].s();
      std::string column=json["column"].s();
      auto parsed=parseCSV(csvData);
      std::vector<FindReplaceRule> rules;
      for(auto& r:json["rules"]) {
        FindReplaceRule rule;
        rule.find=r["find"].s();
        rule.replace=r["replace"].s();
        rule.matchType=r["matchType"].s();
        rule.caseSensitive=r["caseSensitive"].b();
        rules.push_back(rule);
      }
      std::vector<std::string> headers=parsed.empty()?std::vector<std::string>():parsed[0];
      auto frResult=applyFindReplace(parsed,column,rules,headers);
      std::string csvStr=serializeToCSV(frResult.data);
      crow::json::wvalue resp;
      resp["csvData"]=csvStr;
      resp["totalReplacements"]=frResult.totalReplacements;
      logRequest("POST", "/api/find-replace", 200);
      return crow::response(resp);
    } catch(...) {
      logRequest("POST", "/api/find-replace", 400);
      return crow::response(400);
    }
  });
  CROW_ROUTE(app,"/api/detect-clusters").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/detect-clusters", 429); return crow::response(429);}
    try {
      auto json=crow::json::load(req.body);
      std::string csvData=json["csvData"].s();
      std::string column=json["column"].s();
      double threshold=json["threshold"].d();
      auto parsed=parseCSV(csvData);
      std::vector<std::string> headers=parsed.empty()?std::vector<std::string>():parsed[0];
      auto clResult=detectClusters(parsed,column,threshold,headers);
      crow::json::wvalue resp;
      resp["clusters"]=crow::json::wvalue::list();
      int idx=0;
      for(const auto& c:clResult.clusters) {
        resp["clusters"][idx]["id"]=c.id;
        resp["clusters"][idx]["count"]=c.count;
        resp["clusters"][idx]["values"]=crow::json::wvalue::list();
        int vidx=0;
        for(const auto& v:c.values) {
          resp["clusters"][idx]["values"][vidx++]=v;
        }
        idx++;
      }
      logRequest("POST", "/api/detect-clusters", 200);
      return crow::response(resp);
    } catch(...) {
      logRequest("POST", "/api/detect-clusters", 400);
      return crow::response(400);
    }
  });
  CROW_ROUTE(app,"/api/merge-clusters").methods("POST"_method)
  ([](const crow::request& req){
    if (!checkRateLimit(req.remote_ip_address)) {logRequest("POST", "/api/merge-clusters", 429); return crow::response(429);}
    try {
      auto json=crow::json::load(req.body);
      std::string csvData=json["csvData"].s();
      std::string column=json["column"].s();
      auto parsed=parseCSV(csvData);
      std::vector<MergeMapping> merges;
      for(auto& m:json["merges"]) {
        MergeMapping mm{0,m["mergeInto"].s(),{}};
        if(m.has("values")) {
          for(auto& v:m["values"]) mm.values.push_back(v.s());
        }
        merges.push_back(mm);
      }
      std::vector<std::string> headers=parsed.empty()?std::vector<std::string>():parsed[0];
      auto mcResult=applyClustering(parsed,column,merges,headers);
      std::string csvStr=serializeToCSV(mcResult);
      crow::json::wvalue resp;
      resp["csvData"]=csvStr;
      resp["mergeCount"]=(int)merges.size();
      logRequest("POST", "/api/merge-clusters", 200);
      return crow::response(resp);
    } catch(...) {
      logRequest("POST", "/api/merge-clusters", 400);
      return crow::response(400);
    }
  });
}

