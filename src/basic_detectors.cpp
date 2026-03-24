#include "detectors.h"
#include <algorithm>
#include <set>

int levenshteinDistance(const std::string& s1, const std::string& s2){
  size_t len1=s1.length();
  size_t len2=s2.length();
  std::vector<std::vector<int>> dp(len1+1,std::vector<int>(len2+1,0));
  for(size_t i=0;i<=len1;++i) dp[i][0]=i;
  for(size_t j=0;j<=len2;++j) dp[0][j]=j;
  for(size_t i=1;i<=len1;++i){
    for(size_t j=1;j<=len2;++j){
      if(s1[i-1]==s2[j-1]) dp[i][j]=dp[i-1][j-1];
      else dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});
    }
  }
  return dp[len1][len2];
}

double calculateSimilarity(const std::string& s1, const std::string& s2){
  int distance=levenshteinDistance(s1,s2);
  int maxLen=std::max(s1.length(),s2.length());
  if(maxLen==0) return 1.0;
  return 1.0-(double)distance/maxLen;
}

std::vector<std::vector<bool>> detectMissingValues(const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<bool>> missing;
  for(const auto& row:data){
    std::vector<bool> rowMissing;
    for(const auto& cell:row) rowMissing.push_back(cell.empty());
    missing.push_back(rowMissing);
  }
  return missing;
}

std::vector<bool> detectDuplicates(const std::vector<std::vector<std::string>>& data){
  std::vector<bool> isDuplicate(data.size(),false);
  std::set<std::vector<std::string>> seen;
  for(size_t i=0;i<data.size();++i){
    if(seen.count(data[i])) isDuplicate[i]=true;
    else seen.insert(data[i]);
  }
  return isDuplicate;
}

std::map<std::string, std::vector<std::string>> detectInconsistentValues(
  const std::vector<std::vector<std::string>>& data){
  std::map<std::string, std::vector<std::string>> result;
  if(data.size()<2) return result;
  size_t numCols=data[0].size();
  for(size_t col=0;col<numCols;col++){
    std::vector<std::string> colValues;
    for(size_t row=1;row<data.size();row++){
      if(col<data[row].size()&&!data[row][col].empty()) colValues.push_back(data[row][col]);
    }
    if(colValues.size()<2) continue;
    std::set<std::string> processed;
    for(size_t i=0;i<colValues.size();i++){
      if(processed.count(colValues[i])) continue;
      processed.insert(colValues[i]);
      for(size_t j=i+1;j<colValues.size();j++){
        if(processed.count(colValues[j])) continue;
        int distance=levenshteinDistance(colValues[i],colValues[j]);
        if(distance<=2) result[colValues[i]].push_back(colValues[j]);
      }
    }
  }
  return result;
}

