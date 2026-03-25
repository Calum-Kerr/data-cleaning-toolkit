#include "cleaners.h"
#include "text_utils.h"
#include "detectors.h"
#include <algorithm>
#include <set>

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
      std::string upper=toUpperCase(cell);
      size_t start=upper.find_first_not_of(" \t");
      size_t end=upper.find_last_not_of(" \t");
      if(start!=std::string::npos) newRow.push_back(upper.substr(start,end-start+1));
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
      std::string newCell=toUpperCase(cell);
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

std::vector<std::vector<std::string>> fuzzyDeduplicateRows(
  const std::vector<std::vector<std::string>>& data, double threshold){
  std::vector<std::vector<std::string>> result;
  std::vector<bool> merged(data.size(),false);
  for(size_t i=0;i<data.size();i++){
    if(merged[i]) continue;
    std::vector<std::string> combined=data[i];
    for(size_t j=i+1;j<data.size();j++){
      if(merged[j]) continue;
      double sim=calculateSimilarity(data[i],data[j]);
      if(sim>=threshold) merged[j]=true;
    }
    result.push_back(combined);
  }
  return result;
}

