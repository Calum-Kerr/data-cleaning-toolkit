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

std::vector<std::vector<std::string>> parseCSVRFC4180(const std::string& data){
  std::vector<std::vector<std::string>> result;
  std::vector<std::string> currentRow;
  std::string currentField;
  ParserState state = FIELD_START;
  for(size_t i=0;i<data.length();++i){
    char c=data[i];
    if(state==FIELD_START&&c==QUOTE) state=QUOTED_FIELD;
    else if(state==QUOTED_FIELD&&c==QUOTE) state=ESCAPE_QUOTE;
    else if(state==ESCAPE_QUOTE){
      if(c==QUOTE){currentField+=QUOTE;state=QUOTED_FIELD;}
      else if(c==COMMA||c==NEWLINE){currentRow.push_back(currentField);currentField="";state=(c==COMMA)?FIELD_START:FIELD_END;}
    }
    else if(state==UNQUOTED_FIELD&&(c==COMMA||c==NEWLINE)){
      currentRow.push_back(currentField);
      currentField="";
      state=(c==COMMA)?FIELD_START:FIELD_END;
    }
  }
  return result;
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

