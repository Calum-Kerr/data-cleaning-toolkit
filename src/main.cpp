#include "crow_all.h"
#include "algorithms.h"
#include <iostream>

int main(){
    crow::SimpleApp app;
    DataCleaner cleaner;

    CROW_ROUTE(app,"/")
    ([](){
        return "Data Cleaning Toolkit API is running :)";
    });

    CROW_ROUTE(app,"/api/parse").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        crow::json::wvalue result;
        result["rows"]=parsed.size();
        result["message"]="CSV parsed successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-missing").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto missing=cleaner.detectMissingValues(parsed);
        int missingCount=0;
        for(const auto& row:missing){
            for(bool isMissing:row){
                if(isMissing){
                    ++missingCount;
                }
            }
        }
        crow::json::wvalue result;
        result["missing"]=missingCount;
        result["message"]="Missing values detected successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto duplicates=cleaner.detectDuplicates(parsed);
        int duplicateCount=0;
        for(bool isDup:duplicates){
            if(isDup){
                ++duplicateCount;
            }
        }
        crow::json::wvalue result;
        result["duplicates"]=duplicateCount;
        result["message"]="Duplicate rows detected successfully";
        return crow::response(result);
    });
}