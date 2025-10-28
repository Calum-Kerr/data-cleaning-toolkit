#include "algorithms.h"
#include <sstream>
#include <algorithm>
#include <set>

std::vector<std::vector<std::string>> DataCleaner::parseCSV(const std::string& data){
    /**
     * in this function i will take the data and parse it into a 2d vector of strings by
     * splitting the data on the newline character and then splitting each line on the comma character because
     * the data is in the scv format when a user uploads a file it is read as a string.
     */
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

std::vector<std::vector<bool>> DataCleaner::detectMissingValues(const std::vector<std::vector<std::string>>& data){
    /**
     * now for this function i will check for missing values in the data and return a 2d vector of booleans
     * where true tells us that the value is missing and false tells us that the value is not missing
     */
    std::vector<std::vector<bool>> missing;
    for(const auto& row:data){
        std::vector<bool> rowMissing;
        for(const auto& cell:row){
            rowMissing.push_back(cell.empty());
        }
        missing.push_back(rowMissing);
    }
    return missing;
}

std::vector<bool> DataCleaner::detectDuplicates(const std::vector<std::vector<std::string>>& data){
    /**
     * here i will check for dupes in the data and returns a vector of booleans where true tells us that the row is a duplicate
     */
    std::vector<bool> isDuplicate(data.size(),false);
    std::set<std::vector<std::string>> seen;
    for(size_t i=0;i<data.size();++i){
        if(seen.count(data[i])){
            isDuplicate[i]=true;
        }else{
            seen.insert(data[i]);
        }
    }
    return isDuplicate;
}

std::vector<std::vector<std::string>> DataCleaner::cleanData(const std::vector<std::vector<std::string>>& data){

}