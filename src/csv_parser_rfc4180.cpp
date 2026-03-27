#include "csv_parser.h"

// RFC 4180 CSV parsing state machine. see: https://tools.ietf.org/html/rfc4180
// FIELD_START: beginning of field, check for quote or content
// UNQUOTED_FIELD: reading unquoted content, stop at comma or newline
// QUOTED_FIELD: inside quotes, accumulate everything including newlines
// ESCAPE_QUOTE: just saw quote inside quoted field, check if it's an escape ("")
// FIELD_END: field complete, expect comma or newline to proceed to next field
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

ParserState handleUnquotedField(char c, std::vector<std::string>& row,
                                std::string& field){
  if(c==COMMA||c==NEWLINE){completeField(row,field);return(c==COMMA)?FIELD_START:FIELD_END;}
  field+=c;
  return UNQUOTED_FIELD;
}

std::vector<std::vector<std::string>> parseCSVRFC4180(const std::string& data){
  std::vector<std::vector<std::string>> result;
  std::vector<std::string> currentRow;
  std::string currentField;
  ParserState state=FIELD_START;
  for(size_t i=skipUTF8BOM(data);i<data.length();++i){
    char c=data[i];
    if(c==CARRIAGE_RETURN&&i+1<data.length()&&data[i+1]==NEWLINE) continue;
    if(state==FIELD_START) state=handleFieldStart(c,currentRow,currentField);
    else if(state==QUOTED_FIELD) state=handleQuotedField(c);
    else if(state==ESCAPE_QUOTE) state=handleEscapeQuote(c,currentRow,currentField);
    else if(state==UNQUOTED_FIELD) state=handleUnquotedField(c,currentRow,currentField);
    if(state==FIELD_END){completeRow(result,currentRow);state=FIELD_START;}
  }
  if(!currentField.empty()||state==FIELD_END) completeField(currentRow,currentField);
  completeRow(result,currentRow);
  return result;
}
