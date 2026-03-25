#include "text_utils.h"
#include <algorithm>

std::string removeStateSuffixes(const std::string& text){
  std::string result=toUpperCase(text);
  size_t commaPos=result.rfind(',');
  if(commaPos!=std::string::npos)
    result=result.substr(0,commaPos);
  while(!result.empty() && result.back()==' ')
    result.pop_back();
  return result;
}

