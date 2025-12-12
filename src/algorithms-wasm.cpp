#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <emscripten/emscripten.h>

std::vector<std::vector<std::string>> parseCSVInternal(const std::string& data){
    std::vector<std::vector<std::string>> result;
    std::stringstream ss(data);
    std::string line;
    while(std::getline(ss,line)){
        std::vector<std::string> row;
        std::stringstream lineStream(line);
        std::string cell;
        while(std::getline(lineStream,cell,',')){row.push_back(cell);}
        if(!row.empty()){result.push_back(row);}
    }
    return result;
}

extern "C"{
    EMSCRIPTEN_KEEPALIVE
    int parseCSV(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        return parsed.size();
    }
    EMSCRIPTEN_KEEPALIVE
    int detectMissing(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectDuplicates(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        int count=0;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(seen.count(rowStr)){count++;}
            else{seen.insert(rowStr);}
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    int cleanData(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        int cleanedCount=0;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(!seen.count(rowStr)){
                seen.insert(rowStr);
                cleanedCount++;
            }
        }
        return cleanedCount;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* cleanDataString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::set<std::string> seen;
        std::stringstream result;
        for(const auto& row:parsed){
            std::stringstream ss;
            for(size_t i=0;i<row.size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(!seen.count(rowStr)){
                seen.insert(rowStr);
                result<<rowStr<<"\n";
            }
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectWhitespace(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(!cell.empty()&&(cell.front()==' '||cell.back()==' '||cell.front()=='\t'||cell.back()=='\t')){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* trimWhitespaceString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                size_t start=cell.find_first_not_of(" \t");
                size_t end=cell.find_last_not_of(" \t");
                if(start!=std::string::npos){cell=cell.substr(start,end-start+1);}
                else{cell="";}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* toUpperCaseString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){if(c>='a'&&c<='z')c=c-32;}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* toLowerCaseString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                for(char& c:cell){if(c>='A'&&c<='Z')c=c+32;}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
    EMSCRIPTEN_KEEPALIVE
    int detectNullValues(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        int count=0;
        for(const auto& row:parsed){
            for(const auto& cell:row){
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){count++;}
            }
        }
        return count;
    }
    EMSCRIPTEN_KEEPALIVE
    const char* standardiseNullValueString(const char* csvData){
        std::string data(csvData);
        auto parsed=parseCSVInternal(data);
        std::stringstream result;
        for(const auto& row:parsed){
            for(size_t i=0;i<row.size();++i){
                std::string cell=row[i];
                if(cell.empty()||cell=="N/A"||cell=="n/a"||cell=="NA"||cell=="null"||cell=="NULL"||cell=="None"||cell=="NONE"||cell=="-"||cell=="?"){cell="";}
                if(i>0)result<<",";
                result<<cell;
            }
            result<<"\n";
        }
        std::string resultStr=result.str();
        char* cstr=new char[resultStr.length()+1];
        std::strcpy(cstr,resultStr.c_str());
        return cstr;
    }
}