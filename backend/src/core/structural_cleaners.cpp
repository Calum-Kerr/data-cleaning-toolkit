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
  if(data.empty()) return {};
  std::vector<std::vector<std::string>> result;
  result.push_back(data[0]); // preserve header row
  std::unordered_map<uint64_t, std::vector<size_t>> seen;
  for(size_t i=1;i<data.size();i++){
    const auto& row=data[i];
    uint64_t h=fnv1a(row);
    auto& indices=seen[h];
    bool duplicate=false;
    for(size_t idx:indices){
      if(result[idx]==row){duplicate=true;break;}
    }
    if(!duplicate){
      indices.push_back(result.size());
      result.push_back(row);
    }
  }
  return result;
}

static std::string collapseWhitespace(const std::string& s){
  std::string out;
  bool inSpace=false;
  for(char c : s){
    if(c==' ' || c=='\t' || c=='\r' || c=='\n'){
      if(!inSpace){ out+=' '; inSpace=true; }
    }else{
      out+=c;
      inSpace=false;
    }
  }
  // trim trailing space from collapse
  while(!out.empty() && out.back()==' ') out.pop_back();
  // trim leading space from collapse
  size_t lead=0;
  while(lead<out.size() && out[lead]==' ') lead++;
  return out.substr(lead);
}

std::vector<std::vector<std::string>> trimWhitespace(const std::vector<std::vector<std::string>>& data){
  std::vector<std::vector<std::string>> result;
  for(const auto& row:data){
    std::vector<std::string> newRow;
    for(const auto& cell:row){
      // trim edges of \r, \n, \t, space
      size_t start=cell.find_first_not_of(" \t\r\n");
      size_t end=cell.find_last_not_of(" \t\r\n");
      std::string trimmed;
      if(start!=std::string::npos) trimmed=cell.substr(start,end-start+1);
      else trimmed="";
      // collapse internal whitespace runs
      newRow.push_back(collapseWhitespace(trimmed));
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
      if(caseType=="upper") newRow.push_back(toUpperCase(cell));
      else if(caseType=="lower") newRow.push_back(toLowerCase(cell));
      else newRow.push_back(cell);
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

