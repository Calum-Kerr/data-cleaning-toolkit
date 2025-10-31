#include "crow_all.h"
#include "algorithms.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

int main(){
    crow::SimpleApp app;
    DataCleaner cleaner;

    const char* port_env=std::getenv("PORT");
    int port=port_env?std::stoi(port_env):8080;

    CROW_ROUTE(app,"/")
    ([](){
        return "Data Cleaning Toolkit API is running! If this works, the web app is available at /app in the address bar up top :)";
    });

    CROW_ROUTE(app,"/app")
    ([](){
        std::ifstream file("frontend/index.html");
        std::stringstream buffer;
        buffer<<file.rdbuf();
        return crow::response(buffer.str());
    });

    CROW_ROUTE(app,"/algorithms.js")
    ([](){
        std::ifstream file ("frontend/algorithms.js");
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","application/javascript");
        return response;
    });

    CROW_ROUTE(app, "/algorithms.wasm")
    ([](){
        std::ifstream file("frontend/algorithms.wasm",std::ios::binary);
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","application/wasm");
        return response;
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
    

    app.port(port).multithreaded().run();
}