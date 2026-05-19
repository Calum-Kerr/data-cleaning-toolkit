#include "text_utils.h"
#include <algorithm>
#include <sstream>

std::string removeStateSuffixes(const std::string& text){
  std::string result=toUpperCase(text);
  size_t commaPos=result.rfind(',');
  if(commaPos!=std::string::npos)
    result=result.substr(0,commaPos);
  while(!result.empty() && result.back()==' ')
    result.pop_back();
  return result;
}

std::string removeDuplicateWords(const std::string& text){
  std::string upper=toUpperCase(text);
  std::istringstream iss(upper);
  std::string word,result;
  std::string lastWord;
  while(iss>>word){
    if(word!=lastWord){
      if(!result.empty()) result+=" ";
      result+=word;
      lastWord=word;
    }
  }
  return result;
}

