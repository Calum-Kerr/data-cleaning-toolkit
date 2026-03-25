#include "detectors.h"
#include <algorithm>
#include <set>
#include <cmath>

std::vector<int> detectOutliers(const std::vector<std::vector<std::string>>& data){
  std::vector<int> outlierRows;
  if(data.size()<2) return outlierRows;
  auto isNumeric=[](const std::string& str)->bool{
    if(str.empty()) return false;
    size_t start=0;
    if(str[0]=='-'||str[0]=='+') start=1;
    if(start>=str.length()) return false;
    bool hasDecimal=false;
    for(size_t i=start;i<str.length();i++){
      if(str[i]=='.'){
        if(hasDecimal) return false;
        hasDecimal=true;
      }else if(str[i]<'0'||str[i]>'9') return false;
    }
    return true;
  };
  size_t numCols=data[0].size();
  std::set<int> outlierSet;
  for(size_t col=0;col<numCols;col++){
    std::vector<double> values;
    std::vector<int> rowIndices;
    for(size_t row=1;row<data.size();row++){
      if(col<data[row].size()&&isNumeric(data[row][col])){
        values.push_back(std::stod(data[row][col]));
        rowIndices.push_back(row);
      }
    }
    if(values.size()<4) continue;
    std::vector<double> sorted=values;
    std::sort(sorted.begin(),sorted.end());
    size_t n=sorted.size();
    double q1=sorted[n/4];
    double q3=sorted[3*n/4];
    double iqr=q3-q1;
    double lower=q1-1.5*iqr;
    double upper=q3+1.5*iqr;
    for(size_t i=0;i<values.size();i++){
      if(values[i]<lower||values[i]>upper) outlierSet.insert(rowIndices[i]);
    }
  }
  for(int row : outlierSet) outlierRows.push_back(row);
  return outlierRows;
}

double calculateRowSimilarity(const std::vector<std::string>& r1,
  const std::vector<std::string>& r2){
  if(r1.empty() || r2.empty()) return 0.0;
  double totalSim=0.0;
  size_t minSize=std::min(r1.size(),r2.size());
  for(size_t i=0;i<minSize;i++){
    totalSim+=calculateSimilarity(r1[i],r2[i]);
  }
  return totalSim/minSize;
}
