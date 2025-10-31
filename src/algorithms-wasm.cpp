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
    
}