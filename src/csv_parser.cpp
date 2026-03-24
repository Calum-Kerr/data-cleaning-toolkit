#include "csv_parser.h"

std::vector<std::string> parseCSVLine(const std::string& line){
  std::vector<std::string> row;
  std::string cell;
  bool inQuotes=false;
  for(size_t i=0;i<line.length();++i){
    char c=line[i];
    if(c=='"') inQuotes=!inQuotes;
    else if(c==',' && !inQuotes){
      row.push_back(cell);
      cell="";
    }else cell+=c;
  }
  row.push_back(cell);
  return row;
}

std::vector<std::vector<std::string>> parseCSV(const std::string& data){
  return {};
}

