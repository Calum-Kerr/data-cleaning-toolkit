#include "structural_cleaners.h"
#include "text_normalisation.h"
#include "string_issue_detectors.h"
#include <algorithm>
#include <unordered_map>
#include <cstdint>

uint64_t fnv1a(const std::vector<std::string>& row){
  uint64_t hash=14695981039346656037ULL;
  const uint64_t FNV_PRIME=1099511628211ULL;
  for(const auto& cell:row){
    for(char c:cell)
      hash=(hash^(uint8_t)c)*FNV_PRIME;
    // Cell boundary marker: separator byte plus cell length, so rows like
    // ["ab","c"] and ["a","bc"] cannot produce the same byte stream.
    hash=(hash^(uint8_t)0x1F)*FNV_PRIME;
    uint64_t len=cell.size();
    for(int i=0;i<8;i++)
      hash=(hash^(uint8_t)(len>>(i*8)))*FNV_PRIME;
  }
  return hash;
}

std::vector<std::vector<std::string>> removeDuplicates(const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<std::string>> result;
  std::unordered_set<uint64_t> seen;
  for(const auto& row:data){
    uint64_t h=fnv1a(row);
    if(!seen.count(h)){
      seen.insert(h);
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

std::vector<std::vector<std::string>> standardiseCase(
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

std::vector<std::vector<std::string>> standardiseNullValuesInData(
  const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<std::string>> result;
  for(const auto& row:data){
    std::vector<std::string> newRow;
    for(const auto& cell:row) newRow.push_back(standardiseNullValues(cell));
    result.push_back(newRow);
  }
  return result;
}

