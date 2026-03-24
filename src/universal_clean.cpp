#include "cleaners.h"
#include "text_utils.h"
#include "detectors.h"
#include <algorithm>
#include <set>
#include <map>

std::map<std::string, std::vector<std::string>> buildFuzzyMatchingGroups(
  const std::vector<std::string>& columnValues, double threshold=0.75);

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

