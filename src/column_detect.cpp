#include "text_utils.h"
#include <algorithm>

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

