#include "text_utils.h"
#include <algorithm>
#include <sstream>

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

std::string removeDuplicateWords(const std::string& text){
  std::vector<std::string> words;
  std::stringstream ss(text);
  std::string word;
  while(ss >> word) words.push_back(word);
  std::string result;
  for(size_t i=0;i<words.size();++i){
    if(i==0 || words[i]!=words[i-1]){
      if(i>0) result+=" ";
      result+=words[i];
    }
  }
  return result;
}

