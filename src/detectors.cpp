#include "detectors.h"
#include <algorithm>
#include <set>

int levenshteinDistance(const std::string& s1, const std::string& s2){
  size_t m=s1.length();
  size_t n=s2.length();
  std::vector<std::vector<int>> dp(m+1,std::vector<int>(n+1,0));
  for(size_t i=0;i<=m;++i) dp[i][0]=i;
  for(size_t j=0;j<=n;++j) dp[0][j]=j;
  for(size_t i=1;i<=m;++i){
    for(size_t j=1;j<=n;++j){
      if(s1[i-1]==s2[j-1]) dp[i][j]=dp[i-1][j-1];
      else dp[i][j]=1+std::min({dp[i-1][j],dp[i][j-1],dp[i-1][j-1]});
    }
  }
  return dp[m][n];
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

