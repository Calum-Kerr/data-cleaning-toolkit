#include "cleaners.h"
#include <cctype>
#include <algorithm>

bool isNumeric(const std::string& s){
  if(s.empty()) return false;
  for(char c:s) if(!std::isdigit(c)) return false;
  return true;
}

int naturalCompare(const std::string& a, const std::string& b){
  size_t i=0,j=0;
  while(i<a.length() && j<b.length()){
    if(std::isdigit(a[i]) && std::isdigit(b[j])){
      int numA=0,numB=0;
      while(i<a.length() && std::isdigit(a[i]))
        numA=numA*10+(a[i++]-'0');
      while(j<b.length() && std::isdigit(b[j]))
        numB=numB*10+(b[j++]-'0');
      if(numA!=numB) return numA<numB?-1:1;
    }else{
      if(a[i]!=b[j]) return a[i]<b[j]?-1:1;
      i++;j++;
    }
  }
  return a.length()<b.length()?-1:(a.length()>b.length()?1:0);
}

std::vector<std::vector<std::string>> naturalSort(
  const std::vector<std::vector<std::string>>& data, int colIndex){
  auto sorted=data;
  std::sort(sorted.begin(),sorted.end(),
    [colIndex](const auto& a, const auto& b){
      if(colIndex<0 || colIndex>=(int)a.size()) return false;
      return naturalCompare(a[colIndex],b[colIndex])<0;
    });
  return sorted;
}

