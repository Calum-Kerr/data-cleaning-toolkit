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
        while(std::getline(lineStream,cell,',')){
            row.push_back(cell);
        }
        if(!row.empty()){
            result.push_back(row);
        }
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
                if(cell.empty()){
                    count++;
                }
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
            for(size_t=0;i<row,size();++i){
                if(i>0)ss<<",";
                ss<<row[i];
            }
            std::string rowStr=ss.str();
            if(seen.count(rowStr)){
                count++;
            }else{
                seen.insert(rowStr);
            }
        }
        return count;
    }
}