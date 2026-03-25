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

