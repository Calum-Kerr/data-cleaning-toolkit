#include "cleaners.h"
#include "detectors.h"
#include <set>

std::vector<std::vector<std::string>> removeOutliers(
  const std::vector<std::vector<std::string>>& data){
  std::vector<int> outlierRows=detectOutliers(data);
  std::set<int> outlierSet(outlierRows.begin(),outlierRows.end());
  std::vector<std::vector<std::string>> result;
  for(size_t i=0;i<data.size();i++){
    if(outlierSet.find(i)==outlierSet.end()) result.push_back(data[i]);
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
      double sim=calculateRowSimilarity(data[i],data[j]);
      if(sim>=threshold) merged[j]=true;
    }
    result.push_back(combined);
  }
  return result;
}
