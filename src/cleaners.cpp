#include "cleaners.h"
#include "text_utils.h"
#include "detectors.h"
#include <algorithm>
#include <set>
#include <sstream>
#include <map>

std::vector<std::vector<std::string>> removeDuplicates(const std::vector<std::vector<std::string>>& data){
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

std::vector<std::vector<std::string>> trimWhitespace(const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<std::string>> result;
  for(const auto& row:data){
    std::vector<std::string> newRow;
    for(const auto& cell:row){
      size_t start=cell.find_first_not_of(" \t");
      size_t end=cell.find_last_not_of(" \t");
      if(start!=std::string::npos) newRow.push_back(cell.substr(start,end-start+1));
      else newRow.push_back("");
    }
    result.push_back(newRow);
  }
  return result;
}

std::vector<std::vector<std::string>> standardizeCase(
  const std::vector<std::vector<std::string>>& data, const std::string& caseType){
  std::vector<std::vector<std::string>> result;
  for(const auto& row:data){
    std::vector<std::string> newRow;
    for(const auto& cell:row){
      std::string newCell=cell;
      if(caseType=="upper") std::transform(newCell.begin(),newCell.end(),newCell.begin(),::toupper);
      else if(caseType=="lower") std::transform(newCell.begin(),newCell.end(),newCell.begin(),::tolower);
      newRow.push_back(newCell);
    }
    result.push_back(newRow);
  }
  return result;
}

std::vector<std::vector<std::string>> standardizeNullValuesInData(
  const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<std::string>> result;
  for(const auto& row:data){
    std::vector<std::string> newRow;
    for(const auto& cell:row) newRow.push_back(standardizeNullValues(cell));
    result.push_back(newRow);
  }
  return result;
}

std::vector<std::vector<std::string>> removeOutliers(const std::vector<std::vector<std::string>>& data){
  std::vector<int> outlierRows=detectOutliers(data);
  std::set<int> outlierSet(outlierRows.begin(),outlierRows.end());
  std::vector<std::vector<std::string>> result;
  for(size_t i=0;i<data.size();i++){
    if(outlierSet.find(i)==outlierSet.end()) result.push_back(data[i]);
  }
  return result;
}

