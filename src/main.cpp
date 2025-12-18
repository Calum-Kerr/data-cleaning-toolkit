#include "crow_all.h"
#include "algorithms.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>

int main(){
    crow::SimpleApp app;
    DataCleaner cleaner;
    AuditLog auditLog;

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
    ([&cleaner, &auditLog](const crow::request& req){
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
        auditLog.addEntry("Detect Missing", missingCount, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["missing"]=missingCount;
        result["message"]="Missing values detected successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-duplicates").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto duplicates=cleaner.detectDuplicates(parsed);
        int duplicateCount=0;
        for(bool isDup:duplicates){
            if(isDup){
                ++duplicateCount;
            }
        }
        auditLog.addEntry("Detect Duplicates", duplicateCount, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["duplicates"]=duplicateCount;
        result["message"]="Duplicate rows detected successfully";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/clean").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto data=req.body;
        auto parsed=cleaner.parseCSV(data);
        auto cleaned=cleaner.cleanData(parsed);
        int removedRows=parsed.size()-cleaned.size();
        auditLog.addEntry("Remove Duplicates", removedRows, parsed.size(), cleaned.size());
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=cleaned.size();
        result["removedRows"]=removedRows;
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

    CROW_ROUTE(app,"/api/trim-whitespace").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                size_t start=cell.find_first_not_of(" \t");
                size_t end=cell.find_last_not_of(" \t");
                if(start!=std::string::npos){
                    cell=cell.substr(start,end-start+1);
                    count++;
                }else{
                    cell="";
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("Trim Whitespace", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="whitespace trimmed";
        result["cellsTrimmed"]=count;
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/to-uppercase").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){
                    if(c>='a'&&c<='z'){c=c-32;count++;}
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("To Uppercase", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="converted to uppercase";
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/to-lowercase").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){
                    if(c>='A'&&c<='Z'){c=c+32;count++;}
                }
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("To Lowercase", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="converted to lowercase";
        result["cleaned"]=cleaned.str();
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

    CROW_ROUTE(app,"/api/detect-null-values").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){count++;}
            }
        }
        crow::json::wvalue result;
        result["message"]="null values detected";
        result["cellsWithNullValues"]=count;
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/standardise-null-values").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        int count=0;
        std::stringstream cleaned;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){cell="";count++;}
                if(i>0)cleaned<<",";
                cleaned<<cell;
            }
            cleaned<<"\n";
        }
        auditLog.addEntry("Standardise Null Values", count, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["message"]="null values standardised";
        result["cellsStandardised"]=count;
        result["cleaned"]=cleaned.str();
        result["mode"]="api";
        return result;
    });

    CROW_ROUTE(app,"/api/get-audit-log").methods("GET"_method)
    ([&auditLog](){
        crow::json::wvalue result;
        result["operations"]=crow::json::wvalue::list();
        int idx=0;
        for(const auto& entry:auditLog.entries){
            result["operations"][idx]["operationName"]=entry.operationName;
            result["operations"][idx]["cellsAffected"]=entry.cellsAffected;
            result["operations"][idx]["rowsBefore"]=entry.rowsBefore;
            result["operations"][idx]["rowsAfter"]=entry.rowsAfter;
            result["operations"][idx]["timestamp"]=entry.timestamp;
            idx++;
        }
        return result;
    });

    CROW_ROUTE(app,"/api/detect-outliers").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["outliers"]=0;result["message"]="insufficient data for outlier detection";return crow::response(result);}
        int outlierCount=0;
        size_t numCols=parsed[0].size();
        crow::json::wvalue outlierDetails=crow::json::wvalue::list();
        int detailIdx=0;
        for(size_t col=0;col<numCols;col++){std::vector<double> values;std::vector<int> rowIndices;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()){try{double val=std::stod(parsed[row][col]);values.push_back(val);rowIndices.push_back(row);}catch(...){}}}
            if(values.size()<4)continue;
            std::vector<double>sortedValues=values;
            std::sort(sortedValues.begin(),sortedValues.end());
            size_t n=sortedValues.size();
            double q1=sortedValues[n/4];
            double q3=sortedValues[3*n/4];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
        }
    });

    app.port(port).multithreaded().run();
}