#include "text_utils.h"
#include <algorithm>
#include <sstream>
#include <set>

std::string normalizeWhitespace(const std::string& text){
  std::string result;
  bool inSpace=false;
  for(char c : text){
    if(c==' ' || c=='\t' || c=='\r' || c=='\n'){
      if(!inSpace && !result.empty()){
        result+=' ';
        inSpace=true;
      }
    }else{
      result+=c;
      inSpace=false;
    }
  }
  while(!result.empty() && result.back()==' ') result.pop_back();
  return result;
}

std::string normalizePunctuation(const std::string& text){
  std::string result;
  for(size_t i=0; i<text.length(); ++i){
    char c=text[i];
    if(c=='\u2013' || c=='\u2014' || c=='\u2010') result+='-';
    else if(c=='\u2018' || c=='\u2019' || c=='\u201C' || c=='\u201D') result+='\'';
    else if(c!='.') result+=c;
  }
  return result;
}

std::string standardizeNullValues(const std::string& text){
  std::string trimmed=normalizeWhitespace(text);
  if(trimmed.empty()) return "";
  if(trimmed=="N/A" || trimmed=="n/a" || trimmed=="NA" || trimmed=="na") return "";
  if(trimmed=="null" || trimmed=="NULL" || trimmed=="Null") return "";
  if(trimmed=="none" || trimmed=="NONE" || trimmed=="None") return "";
  if(trimmed=="-" || trimmed=="?" || trimmed=="~") return "";
  if(trimmed=="(empty)" || trimmed=="EMPTY") return "";
  return trimmed;
}

std::string removeStateSuffixes(const std::string& text){
  std::string result=text;
  size_t commaPos=result.rfind(',');
  if(commaPos != std::string::npos) result=result.substr(0, commaPos);
  std::string upper=result;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  const std::vector<std::string> patterns={
    " ALABAMA", " ALASKA", " ARIZONA", " ARKANSAS", " CALIFORNIA", " COLORADO",
    " CONNECTICUT", " DELAWARE", " FLORIDA", " GEORGIA", " HAWAII", " IDAHO",
    " ILLINOIS", " INDIANA", " IOWA", " KANSAS", " KENTUCKY", " LOUISIANA",
    " MAINE", " MARYLAND", " MASSACHUSETTS", " MICHIGAN", " MINNESOTA",
    " MISSISSIPPI", " MISSOURI", " MONTANA", " NEBRASKA", " NEVADA",
    " NEW HAMPSHIRE", " NEW JERSEY", " NEW MEXICO", " NORTH CAROLINA",
    " NORTH DAKOTA", " OHIO", " OKLAHOMA", " OREGON", " PENNSYLVANIA",
    " RHODE ISLAND", " SOUTH CAROLINA", " SOUTH DAKOTA", " TENNESSEE", " TEXAS",
    " UTAH", " VERMONT", " VIRGINIA", " WEST VIRGINIA",
    " WISCONSIN", " WYOMING", " DISTRICT OF COLUMBIA", " US", " USA", " UNITED STATES"
  };
  for(const auto& pattern : patterns){
    size_t pos=upper.rfind(pattern);
    if(pos != std::string::npos && pos + pattern.length()==upper.length()){
      result=result.substr(0, pos);
      upper=upper.substr(0, pos);
    }
  }
  while(!result.empty() && (result.back()==' ' || result.back()=='\t')) result.pop_back();
  return result;
}

std::string removeDuplicateWords(const std::string& text){
  std::vector<std::string> words;
  std::stringstream ss(text);
  std::string word;
  while(ss >> word) words.push_back(word);
  std::vector<std::string> result;
  for(size_t i=0; i<words.size(); ++i){
    if(i==0 || words[i] != words[i-1]) result.push_back(words[i]);
  }
  std::string output;
  for(size_t i=0; i<result.size(); ++i){
    if(i > 0) output+=" ";
    output+=result[i];
  }
  return output;
}

bool isNumericColumn(const std::vector<std::string>& columnValues, int sampleSize){
  int numericCount=0;
  int sampleCount=std::min((int)columnValues.size(), sampleSize);
  for(int i=0; i<sampleCount; ++i){
    std::string cell=columnValues[i];
    if(cell.empty()) continue;
    char* endptr;
    strtod(cell.c_str(), &endptr);
    if(*endptr=='\0') numericCount++;
  }
  return numericCount < (sampleCount * 0.8);
}

std::vector<int> detectTextColumns(const std::vector<std::vector<std::string>>& parsed){
  std::vector<int> textColumns;
  if(parsed.empty()) return textColumns;
  int numColumns=parsed[0].size();
  for(int col=0; col<numColumns; ++col){
    std::vector<std::string> columnValues;
    for(size_t row=1; row<parsed.size(); ++row){
      if(col < (int)parsed[row].size()) columnValues.push_back(parsed[row][col]);
    }
    if(isNumericColumn(columnValues)) textColumns.push_back(col);
  }
  return textColumns;
}

