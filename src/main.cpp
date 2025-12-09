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

    CROW_ROUTE(app,"/manifest.json")
    ([](){
        std::ifstream file("frontend/manifest.json");
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","application/json");
        return response;
    });

    CROW_ROUTE(app,"/service-worker.js")
    ([](){
        std::ifstream file("frontend/service-worker.js");
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","application/javascript");
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

    CROW_ROUTE(app,"/api/detect-whitespace").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(!cell.empty()&&(cell.front()==' '||cell.back()==' '||cell.front()=='\t'||cell.back()=='\t')){
                    count++;
                }
            }
        }
        crow::json::wvalue result;
        result["message"]="whitespace detected";
        result["cellsWithWhitespace"]=count;
        result["mode"]="api";
        return result;
    });
    
    CROW_ROUTE(app,"/favicon.ico")
    ([](){
        std::ifstream file("frontend/favicon.ico",std::ios::binary);
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","image/x-icon");
        return response;
    });

    CROW_ROUTE(app,"/android-chrome-192x192.png")
    ([](){
        std::ifstream file("frontend/android-chrome-192x192.png",std::ios::binary);
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","image/png");
        return response;
    });

    CROW_ROUTE(app,"/android-chrome-512x512.png")
    ([](){
        std::ifstream file("frontend/android-chrome-512x512.png",std::ios::binary);
        std::stringstream buffer;
        buffer<<file.rdbuf();
        auto response=crow::response(buffer.str());
        response.add_header("Content-Type","image/png");
        return response;
    });

    app.port(port).multithreaded().run();
}