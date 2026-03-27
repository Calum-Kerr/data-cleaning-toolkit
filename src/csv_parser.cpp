#include "csv_parser.h"
#include <sstream>

enum ParserState {FIELD_START, UNQUOTED_FIELD, QUOTED_FIELD, ESCAPE_QUOTE, FIELD_END};
const char QUOTE = '"', COMMA = ',', NEWLINE = '\n', CARRIAGE_RETURN = '\r';

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
  std::vector<std::vector<std::string>> result;
  std::stringstream ss(data);
  std::string line;
  while(std::getline(ss,line)){
    if(!line.empty()) result.push_back(parseCSVLine(line));
  }
  return result;
}

