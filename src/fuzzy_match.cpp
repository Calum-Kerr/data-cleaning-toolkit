#include "cleaners.h"
#include "text_utils.h"
#include "detectors.h"
#include <algorithm>
#include <set>
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

static std::map<std::string, std::vector<std::string>> buildFuzzyMatchingGroups(
  const std::vector<std::string>& columnValues, double threshold=0.75){
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

static std::map<std::string, std::string> createFuzzyMatchingMapping(
  const std::vector<std::string>& columnValues, double threshold=0.75){
  auto groups=buildFuzzyMatchingGroups(columnValues,threshold);
  std::map<std::string, std::string> mapping;
  std::set<std::string> allUnique;
  for(const auto& val:columnValues){
    if(!val.empty()) allUnique.insert(val);
  }
  for(const auto& val:allUnique) mapping[val]=val;
  for(const auto& pair:groups){
    mapping[pair.first]=pair.first;
    for(const auto& val:pair.second) mapping[val]=pair.first;
  }
  return mapping;
}

UniversalCleaningResult universalTextCleaning(
  const std::vector<std::vector<std::string>>& parsed,
  double fuzzyThreshold, bool removeDuplicateRows){
  UniversalCleaningResult result;
  result.duplicateRowsRemoved=0;
  if(parsed.empty()) return result;
  auto textColumns=detectTextColumns(parsed);
  result.operationsLog.push_back("Detected "+std::to_string(textColumns.size())+" text columns");
  for(int col:textColumns){
    std::vector<std::string> columnValues;
    for(size_t row=1;row<parsed.size();++row){
      if(col<(int)parsed[row].size()){
        std::string val=parsed[row][col];
        std::transform(val.begin(),val.end(),val.begin(),::toupper);
        columnValues.push_back(val);
      }
    }
    std::set<std::string> uniqueSet(columnValues.begin(),columnValues.end());
    if(uniqueSet.size()<=500){
      auto mapping=createFuzzyMatchingMapping(columnValues,fuzzyThreshold);
      result.columnMappings[col]=mapping;
      std::set<std::string> mergedUnique;
      for(const auto& val:columnValues){
        if(!val.empty()) mergedUnique.insert(mapping[val]);
      }
      int mergedCount=uniqueSet.size()-mergedUnique.size();
      result.mergedCountPerColumn[col]=mergedCount;
      if(mergedCount>0){
        result.operationsLog.push_back("Column "+std::to_string(col)+": merged "+std::to_string(mergedCount)+" values");
      }
    }else{
      result.operationsLog.push_back("Column "+std::to_string(col)+": skipped fuzzy matching (too many unique values: "+std::to_string(uniqueSet.size())+")");
    }
  }
  std::set<std::vector<std::string>> seenRows;
  for(size_t i=0;i<parsed.size();++i){
    auto row=parsed[i];
    for(size_t j=0;j<row.size();++j){
      std::string cell=row[j];
      bool isTextCol=false;
      for(int col:textColumns){
        if(col==(int)j){
          isTextCol=true;
          break;
        }
      }
      if(isTextCol){
        cell=standardizeNullValues(cell);
        if(!cell.empty()){
          cell=normalizePunctuation(cell);
          cell=normalizeWhitespace(cell);
          std::string cellUpper=cell;
          std::transform(cellUpper.begin(),cellUpper.end(),cellUpper.begin(),::toupper);
          if(result.columnMappings[j].count(cellUpper)){
            cell=result.columnMappings[j][cellUpper];
          }else{
            cell=cellUpper;
          }
        }
      }
      row[j]=cell;
    }
    if(removeDuplicateRows){
      if(!seenRows.count(row)){
        seenRows.insert(row);
        result.cleanedData.push_back(row);
      }else{
        result.duplicateRowsRemoved++;
      }
    }else{
      result.cleanedData.push_back(row);
    }
  }
  if(removeDuplicateRows&&result.duplicateRowsRemoved>0){
    result.operationsLog.push_back("Removed "+std::to_string(result.duplicateRowsRemoved)+" duplicate rows");
  }
  return result;
}

