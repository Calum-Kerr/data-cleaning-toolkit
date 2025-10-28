#include "algorithms.h"
#include <sstream>
#include <algorithm>
#include <set>

std::vector<std::vector<std::string>> DataCleaner::parseCSV(const std::string& data){
    /**
     * in this functio i will take the data and parse it into a 2d vector of strings by
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

}

std::vector<bool> DataCleaner::detectDuplicates(const std::vector<std::vector<std::string>>& data){

}

std::vector<std::vector<std::string>> DataCleaner::cleanData(const std::vector<std::vector<std::string>>& data){

}