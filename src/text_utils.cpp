#include "text_utils.h"
#include <algorithm>

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
  for(char c : text){
    if(c!='.') result+=c;
  }
  return result;
}

std::string standardizeNullValues(const std::string& text){
  std::string trimmed=normalizeWhitespace(text);
  if(trimmed.empty()) return "";
  if(trimmed=="N/A" || trimmed=="n/a" || trimmed=="NA") return "";
  if(trimmed=="null" || trimmed=="NULL" || trimmed=="Null") return "";
  if(trimmed=="none" || trimmed=="NONE" || trimmed=="None") return "";
  if(trimmed=="-" || trimmed=="?" || trimmed=="~") return "";
  return trimmed;
}

