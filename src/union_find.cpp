#include "cleaners.h"
#include <map>

class UnionFind {
public:
  std::map<std::string, std::string> parent;
  std::map<std::string, int> rank;
  void makeSet(const std::string& x){
    if(parent.find(x)==parent.end()){
      parent[x]=x;
      rank[x]=0;
    }
  }
  std::string find(const std::string& x){
    if(parent[x]!=x) parent[x]=find(parent[x]);
    return parent[x];
  }
  void unite(const std::string& x, const std::string& y){
    std::string px=find(x);
    std::string py=find(y);
    if(px==py) return;
    if(rank[px]<rank[py]) parent[px]=py;
    else if(rank[px]>rank[py]) parent[py]=px;
    else{
      parent[py]=px;
      rank[px]++;
    }
  }
};

std::map<std::string, std::vector<std::string>> buildFuzzyMatchingGroups(
  const std::vector<std::string>& columnValues, double threshold){
  std::map<std::string, std::vector<std::string>> groups;
  std::map<std::string, int> valueCounts;
  std::set<std::string> uniqueSet;
  for(const auto& val:columnValues){
    if(!val.empty()){
      uniqueSet.insert(val);
      valueCounts[val]++;
    }
  }
  std::vector<std::string> uniqueValues(uniqueSet.begin(),uniqueSet.end());
  UnionFind uf;
  for(const auto& val:uniqueValues) uf.makeSet(val);
  for(size_t i=0;i<uniqueValues.size();++i){
    for(size_t j=i+1;j<uniqueValues.size();++j){
      std::string prep1=uniqueValues[i];
      std::string prep2=uniqueValues[j];
      prep1=removeStateSuffixes(prep1);
      prep1=removeDuplicateWords(prep1);
      prep1=normalizePunctuation(prep1);
      prep1=normalizeWhitespace(prep1);
      prep2=removeStateSuffixes(prep2);
      prep2=removeDuplicateWords(prep2);
      prep2=normalizePunctuation(prep2);
      prep2=normalizeWhitespace(prep2);
      if(prep1.empty()||prep2.empty()) continue;
      int len1=prep1.length();
      int len2=prep2.length();
      int maxLen=std::max(len1,len2);
      int minLen=std::min(len1,len2);
      if(minLen>0&&(maxLen-minLen)>(maxLen*0.75)) continue;
      double similarity=calculateSimilarity(prep1,prep2);
      if(similarity>=threshold) uf.unite(uniqueValues[i],uniqueValues[j]);
    }
  }
  std::map<std::string, std::vector<std::string>> rootGroups;
  for(const auto& val:uniqueValues){
    std::string root=uf.find(val);
    rootGroups[root].push_back(val);
  }
  for(auto& pair:rootGroups){
    std::vector<std::string>& group=pair.second;
    std::string canonical=group[0];
    int maxCount=valueCounts[canonical];
    for(const auto& val:group){
      if(valueCounts[val]>maxCount){
        canonical=val;
        maxCount=valueCounts[val];
      }
    }
    groups[canonical]=group;
  }
  return groups;
}

