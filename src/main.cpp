#include "crow_all.h"
#include "algorithms.h"
#include <iostream>
#include <cstdlib>

int main(){
    crow::SimpleApp app;
    DataCleaner cleaner;

    const char* port_env=std::getenv("PORT");
    int port=port_env?std::stoi(port_env):8080;

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

    CROW_ROUTE(app,"/api/clean").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto cleaned=cleaner.cleanData(parsed);
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=cleaned.size();
        result["removedRows"]=parsed.size()-cleaned.size();
        result["message"]="Data cleaned successfully";
        return crow::response(result);
    });
    

    const char* port_env=std::getenv("PORT");
    int port=port_env?std::stoi(port_env):8080;
    app.port(port).multithreaded().run();
}