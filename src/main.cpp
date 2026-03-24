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
  app.port(8080).multithreaded().run();
}

