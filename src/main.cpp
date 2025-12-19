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
        if(parsed.size()<2){crow::json::wvalue result; result["outliers"]=0;result["rowsWithOutliers"]=0;result["message"]="insufficient data for outlier detection";return crow::response(result);}
        auto isNumericStr=[](const std::string& str)->bool{
            if(str.empty())return false;
            size_t start=0;
            if(str[0]=='-'||str[0]=='+')start=1;
            if(start>=str.length())return false;
            bool hasDecimal=false;
            for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}
            return true;
        };
        int outlierCount=0;
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        struct OutlierDetail{int row;int col;double val;double lower;double upper;};
        std::vector<OutlierDetail> outlierDetails;
        for(size_t col=0;col<numCols;col++){
            std::vector<double> values;
            std::vector<int> rowIndices;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumericStr(parsed[row][col])){double val=std::stod(parsed[row][col]);values.push_back(val);rowIndices.push_back(row);}}
            if(values.size()<4)continue;
            std::vector<double> sortedValues=values;
            std::sort(sortedValues.begin(),sortedValues.end());
            size_t n=sortedValues.size();
            double q1=sortedValues[n/4];
            double q3=sortedValues[3*n/4];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<values.size();i++){if(values[i]<lower||values[i]>upper){outlierCount++;outlierRows.insert(rowIndices[i]);outlierDetails.push_back({rowIndices[i],(int)col,values[i],lower,upper});}}
        }
        crow::json::wvalue result;
        result["outliers"]=outlierCount;
        result["rowsWithOutliers"]=(int)outlierRows.size();
        result["message"]="outlier detection completed";
        result["mode"]="api";
        result["details"]=crow::json::wvalue::list();
        for(size_t i=0;i<outlierDetails.size();i++){
            result["details"][i]["row"]=outlierDetails[i].row;
            result["details"][i]["column"]=outlierDetails[i].col;
            result["details"][i]["value"]=outlierDetails[i].val;
            result["details"][i]["lower"]=outlierDetails[i].lower;
            result["details"][i]["upper"]=outlierDetails[i].upper;
        }
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/remove-outliers").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["originalRows"]=parsed.size();result["cleanedRows"]=parsed.size();result["removedRows"]=0;result["message"]="insufficient data for outlier removal";return crow::response(result);}
        auto isNumericStr=[](const std::string& str)->bool{
            if(str.empty())return false;
            size_t start=0;
            if(str[0]=='-'||str[0]=='+')start=1;
            if(start>=str.length())return false;
            bool hasDecimal=false;
            for(size_t i=start;i<str.length();i++){if(str[i]=='.'){if(hasDecimal)return false;hasDecimal=true;}else if(str[i]<'0'||str[i]>'9')return false;}
            return true;
        };
        std::set<int> outlierRows;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::vector<double> values;
            std::vector<int> rowIndices;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&isNumericStr(parsed[row][col])){double val=std::stod(parsed[row][col]);values.push_back(val);rowIndices.push_back(row);}}
            if(values.size()<4)continue;
            std::vector<double> sortedValues=values;
            std::sort(sortedValues.begin(),sortedValues.end());
            size_t n=sortedValues.size();
            double q1=sortedValues[n/4];
            double q3=sortedValues[3*n/4];
            double iqr=q3-q1;
            double lower=q1-1.5*iqr;
            double upper=q3+1.5*iqr;
            for(size_t i=0;i<values.size();i++){if(values[i]<lower||values[i]>upper){outlierRows.insert(rowIndices[i]);}}
        }
        std::stringstream cleanedCSV;
        for(size_t i=0;i<parsed.size();i++){
            if(outlierRows.find(i)==outlierRows.end()){
                for(size_t j=0;j<parsed[i].size();j++){if(j>0)cleanedCSV<<",";cleanedCSV<<parsed[i][j];}
                cleanedCSV<<"\n";
            }
        }
        int removedRows=outlierRows.size();
        int cleanedRows=parsed.size()-removedRows;
        auditLog.addEntry("Remove Outliers", removedRows, parsed.size(), cleanedRows);
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=cleanedRows;
        result["removedRows"]=removedRows;
        result["cleaned"]=cleanedCSV.str();
        result["message"]="outliers removed successfully";
        result["mode"]="api";
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/detect-inconsistent-values").methods("POST"_method)
    ([&cleaner](const crow::request& req){
        std::string csvData=req.body;
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["inconsistentCount"]=0;result["message"]="insufficient data for inconsistency detection";return crow::response(result);}
        auto levenshteinDist=[](const std::string& s1,const std::string& s2)->int{
            size_t m=s1.length();
            size_t n=s2.length();
            std::vector<std::vector<int>> dp(m+1,std::vector<int>(n+1,0));
            for(size_t i=0;i<=m;i++)dp[i][0]=i;
            for(size_t j=0;j<=n;j++)dp[0][j]=j;
            for(size_t i=1;i<=m;i++){for(size_t j=1;j<=n;j++){if(s1[i-1]==s2[j-1]){dp[i][j]=dp[i-1][j-1];}else{dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});}}}
            return dp[m][n];
        };
        auto toLower=[](const std::string& str)->std::string{std::string result=str;std::transform(result.begin(),result.end(),result.begin(),::tolower);return result;};
        auto isNameColumn=[&toLower](const std::string& colName)->bool{std::string lower=toLower(colName);const std::vector<std::string> nameKeywords={"player","name","first name","last name","full name","employee","customer","person","author","contact","user","username","email","phone","address","street","city","country","state","zip","postal"};for(const auto& keyword:nameKeywords){if(lower.find(keyword)!=std::string::npos){return true;}}return false;};
        int inconsistentCount=0;
        std::vector<std::string> skippedNameColumns;
        std::map<std::string,std::map<std::string,std::vector<std::string>>> suggestedMappings;
        size_t numCols=parsed[0].size();
        for(size_t col=0;col<numCols;col++){
            std::string colName=parsed[0][col];
            if(isNameColumn(colName)){skippedNameColumns.push_back(colName);continue;}
            std::vector<std::string> colValues;
            for(size_t row=1;row<parsed.size();row++){if(col<parsed[row].size()&&!parsed[row][col].empty()){colValues.push_back(parsed[row][col]);}}
            if(colValues.size()<2)continue;
            std::set<std::string> processedValues;
            for(size_t i=0;i<colValues.size();i++){
                if(processedValues.count(colValues[i]))continue;
                processedValues.insert(colValues[i]);
                for(size_t j=i+1;j<colValues.size();j++){
                    if(processedValues.count(colValues[j]))continue;
                    int distance=levenshteinDist(colValues[i],colValues[j]);
                    if(distance<=2){
                        inconsistentCount++;
                        if(suggestedMappings[colName].find(colValues[i])==suggestedMappings[colName].end()){suggestedMappings[colName][colValues[i]]=std::vector<std::string>();}
                        suggestedMappings[colName][colValues[i]].push_back(colValues[j]);
                    }
                }
            }
        }
        crow::json::wvalue result;
        result["inconsistentCount"]=inconsistentCount;
        result["message"]="inconsistency detection completed";
        result["mode"]="api";
        result["suggestedMappings"]=crow::json::wvalue::object();
        for(auto& colEntry:suggestedMappings){result["suggestedMappings"][colEntry.first]=crow::json::wvalue::object();for(auto& valEntry:colEntry.second){result["suggestedMappings"][colEntry.first][valEntry.first]=valEntry.second[0];}}
        if(!skippedNameColumns.empty()){result["warning"]="name columns excluded from detection to protect data integrity";std::string colsJson="[";for(size_t i=0;i<skippedNameColumns.size();i++){if(i>0)colsJson+=",";colsJson+="\""+skippedNameColumns[i]+"\"";}colsJson+="]";result["skippedColumns"]=crow::json::load(colsJson);}
        return crow::response(result);
    });

    CROW_ROUTE(app,"/api/standardise-values").methods("POST"_method)
    ([&cleaner, &auditLog](const crow::request& req){
        auto body=crow::json::load(req.body);
        if(!body){crow::json::wvalue result; result["message"]="invalid request body";return crow::response(400);}
        std::string csvData=body["csvData"].s();
        std::string mappingsJson=body["mappings"].s();
        auto parsed=cleaner.parseCSV(csvData);
        if(parsed.size()<2){crow::json::wvalue result; result["originalRows"]=parsed.size();result["cleanedRows"]=parsed.size();result["valuesStandardised"]=0;result["message"]="insufficient data for standardisation";return crow::response(result);}
        std::map<std::string,std::map<std::string,std::string>> columnMappings;
        size_t pos=0;
        std::string currentCol;
        while(pos<mappingsJson.length()){
            if(mappingsJson[pos]=='"'){
                size_t endQuote=mappingsJson.find('"',pos+1);
                if(endQuote!=std::string::npos){
                    std::string key=mappingsJson.substr(pos+1,endQuote-pos-1);
                    pos=endQuote+1;
                    while(pos<mappingsJson.length()&&(mappingsJson[pos]==':'||mappingsJson[pos]==' '||mappingsJson[pos]=='\t'||mappingsJson[pos]=='\n')){pos++;}
                    if(pos<mappingsJson.length()&&mappingsJson[pos]=='{'){
                        currentCol=key;
                        pos++;
                        while(pos<mappingsJson.length()&&mappingsJson[pos]!='}'){
                            if(mappingsJson[pos]=='"'){
                                size_t keyEnd=mappingsJson.find('"',pos+1);
                                if(keyEnd!=std::string::npos){
                                    std::string fromVal=mappingsJson.substr(pos+1,keyEnd-pos-1);
                                    pos=keyEnd+1;
                                    while(pos<mappingsJson.length()&&(mappingsJson[pos]==':'||mappingsJson[pos]==' '||mappingsJson[pos]=='\t'||mappingsJson[pos]=='\n')){pos++;}
                                    if(pos<mappingsJson.length()&&mappingsJson[pos]=='"'){
                                        size_t valEnd=mappingsJson.find('"',pos+1);
                                        if(valEnd!=std::string::npos){
                                            std::string toVal=mappingsJson.substr(pos+1,valEnd-pos-1);
                                            columnMappings[currentCol][fromVal]=toVal;
                                            pos=valEnd+1;
                                        }else{pos++;}
                                    }else{pos++;}
                                }else{pos++;}
                            }else{pos++;}
                        }
                    }else{pos++;}
                }else{pos++;}
            }else{pos++;}
        }
        std::stringstream cleanedCSV;
        int valuesStandardised=0;
        for(size_t row=0;row<parsed.size();row++){
            for(size_t col=0;col<parsed[row].size();col++){
                std::string cell=parsed[row][col];
                if(row>0&&col<parsed[0].size()){std::string colName=parsed[0][col];if(columnMappings.count(colName)&&columnMappings[colName].count(cell)){cell=columnMappings[colName][cell];valuesStandardised++;}}
                if(col>0)cleanedCSV<<",";
                cleanedCSV<<cell;
            }
            cleanedCSV<<"\n";
        }
        auditLog.addEntry("Standardise Values", valuesStandardised, parsed.size(), parsed.size());
        crow::json::wvalue result;
        result["originalRows"]=parsed.size();
        result["cleanedRows"]=parsed.size();
        result["valuesStandardised"]=valuesStandardised;
        result["cleaned"]=cleanedCSV.str();
        result["message"]="values standardised successfully";
        result["mode"]="api";
        return crow::response(result);
    });

    app.port(port).multithreaded().run();
}