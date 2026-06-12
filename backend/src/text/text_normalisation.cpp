#include "text_normalisation.h"
#include <algorithm>
#include <sstream>

std::string toUpperCase(const std::string& text){
  std::string result=text;
  std::transform(result.begin(),result.end(),result.begin(),::toupper);
  return result;
}

std::string toLowerCase(const std::string& text){
  std::string result=text;
  std::transform(result.begin(),result.end(),result.begin(),::tolower);
  return result;
}

std::string normaliseWhitespace(const std::string& text){
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

std::string normalisePunctuation(const std::string& text){
  std::string result;
  for(char c : text){
    if(c!='.') result+=c;
  }
  return result;
}

std::string standardiseNullValues(const std::string& text){
  std::string trimmed=normaliseWhitespace(text);
  if(trimmed.empty()) return "";
  std::string upper=toUpperCase(trimmed);
  if(upper=="N/A" || upper=="NA") return "";
  if(upper=="NULL") return "";
  if(upper=="NONE" || upper=="NIL") return "";
  if(upper=="MISSING") return "";
  if(upper=="NAN" || upper=="UNDEFINED") return "";
  if(upper=="-" || upper=="--" || upper=="?" || upper=="~") return "";
  return trimmed;
}

