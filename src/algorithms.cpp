#include "algorithms.h"
#include <sstream>
#include <algorithm>
#include <set>
#include <map>

std::vector<std::vector<std::string>> DataCleaner::parseCSV(const std::string& data){
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

std::vector<std::vector<bool>> DataCleaner::detectMissingValues(const std::vector<std::vector<std::string>>& data){
    std::vector<std::vector<bool>> missing;
    for(const auto& row:data){
        std::vector<bool> rowMissing;
        for(const auto& cell:row){rowMissing.push_back(cell.empty());}
        missing.push_back(rowMissing);
    }
    return missing;
}

std::vector<bool> DataCleaner::detectDuplicates(const std::vector<std::vector<std::string>>& data){
    std::vector<bool> isDuplicate(data.size(),false);
    std::set<std::vector<std::string>> seen;
    for(size_t i=0;i<data.size();++i){
        if(seen.count(data[i])){isDuplicate[i]=true;}
        else{seen.insert(data[i]);}
    }
    return isDuplicate;
}

std::vector<std::vector<std::string>> DataCleaner::cleanData(const std::vector<std::vector<std::string>>& data){
    auto cleaned=data;
    auto duplicates=detectDuplicates(cleaned);
    std::vector<std::vector<std::string>> result;
    for(size_t i=0;i<cleaned.size();++i){if(!duplicates[i]){result.push_back(cleaned[i]);}}
    return result;
}

std::map<std::string,int> DataCleaner::profileColumn(const std::vector<std::vector<std::string>>& data, size_t columnIndex){
    std::map<std::string,int> profile;
    if(data.empty()||columnIndex>=data[0].size()){return profile;}
    for(const auto& row:data){
        if(columnIndex<row.size()){
            profile[row[columnIndex]]++;
        }
    }
    return profile;
}

std::vector<std::vector<std::string>> DataCleaner::standardizeColumnCase(const std::vector<std::vector<std::string>>& data, size_t columnIndex, const std::string& caseType){
    auto result=data;
    if(data.empty()||columnIndex>=data[0].size()){return result;}
    for(auto& row:result){
        if(columnIndex<row.size()){
            std::string& cell=row[columnIndex];
            if(caseType=="upper"){
                for(auto& c:cell){c=std::toupper(c);}
            }else if(caseType=="lower"){
                for(auto& c:cell){c=std::tolower(c);}
            }
        }
    }
    return result;
}

std::vector<std::vector<std::string>> DataCleaner::removeEmptyRows(const std::vector<std::vector<std::string>>& data){
    std::vector<std::vector<std::string>> result;
    for(const auto& row:data){
        bool isEmpty=true;
        for(const auto& cell:row){
            if(!cell.empty()){isEmpty=false;break;}
        }
        if(!isEmpty){result.push_back(row);}
    }
    return result;
}

std::vector<std::vector<std::string>> DataCleaner::removeDuplicates(const std::vector<std::vector<std::string>>& data){
    std::vector<std::vector<std::string>> result;
    std::set<std::vector<std::string>> seen;
    for(const auto& row:data){
        if(!seen.count(row)){
            seen.insert(row);
            result.push_back(row);
        }
    }
    return result;
}