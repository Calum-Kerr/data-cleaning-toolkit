#include "csv_parser.h"

enum ParserState {FIELD_START, UNQUOTED_FIELD, QUOTED_FIELD, ESCAPE_QUOTE, FIELD_END};
const char QUOTE = '"', COMMA = ',', NEWLINE = '\n', CARRIAGE_RETURN = '\r';

bool isValidClosedQuote(const std::string& field){
  int quoteCount=0;
  for(char c:field) if(c==QUOTE) ++quoteCount;
  return quoteCount%2==0;
}

size_t skipUTF8BOM(const std::string& data){
  if(data.length()>=3&&(unsigned char)data[0]==0xEF&&
     (unsigned char)data[1]==0xBB&&(unsigned char)data[2]==0xBF) return 3;
  return 0;
}

void completeField(std::vector<std::string>& row, std::string& field){
  row.push_back(field);
  field="";
}

void completeRow(std::vector<std::vector<std::string>>& result,
                 std::vector<std::string>& row){
  if(!row.empty()) result.push_back(row);
  row.clear();
}

ParserState handleFieldStart(char c, std::vector<std::string>& row,
                             std::string& field){
  if(c==COMMA||c==NEWLINE){row.push_back("");return FIELD_START;}
  if(c==QUOTE) return QUOTED_FIELD;
  if(c!=COMMA&&c!=NEWLINE){field+=c;return UNQUOTED_FIELD;}
  return FIELD_START;
}

ParserState handleQuotedField(char c){
  if(c==QUOTE) return ESCAPE_QUOTE;
  return QUOTED_FIELD;
}

ParserState handleEscapeQuote(char c, std::vector<std::string>& row,
                              std::string& field){
  if(c==QUOTE){field+=QUOTE;return QUOTED_FIELD;}
  if(c==COMMA||c==NEWLINE){completeField(row,field);return(c==COMMA)?FIELD_START:FIELD_END;}
  return QUOTED_FIELD;
}

std::vector<std::vector<std::string>> parseCSVRFC4180(const std::string& data){
  std::vector<std::vector<std::string>> result;
  std::vector<std::string> currentRow;
  std::string currentField;
  ParserState state = FIELD_START;
  size_t startIdx=0;
  if(data.length()>=3&&(unsigned char)data[0]==0xEF&&(unsigned char)data[1]==0xBB&&(unsigned char)data[2]==0xBF) startIdx=3;
  for(size_t i=startIdx;i<data.length();++i){
    char c=data[i];
    if(c==CARRIAGE_RETURN&&i+1<data.length()&&data[i+1]==NEWLINE) continue;
    if(state==FIELD_START&&(c==COMMA||c==NEWLINE)){currentRow.push_back("");state=(c==COMMA)?FIELD_START:FIELD_END;}
    else if(state==FIELD_START&&c==QUOTE) state=QUOTED_FIELD;
    else if(state==FIELD_START&&c!=COMMA&&c!=NEWLINE){state=UNQUOTED_FIELD;currentField+=c;}
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
    else if(state==QUOTED_FIELD) currentField+=c;
    else if(state==UNQUOTED_FIELD) currentField+=c;
  }
  if(!currentField.empty()||state==FIELD_END) currentRow.push_back(currentField);
  if(!currentRow.empty()) result.push_back(currentRow);
  return result;
}
