#include "csv_parser.h"

// Parse a single CSV record (RFC 4180): quoted cells may contain commas and
// "" escapes for one literal quote.  Newline characters in the input are
// treated as cell content, never as a record boundary.
std::vector<std::string> parseCSVLine(const std::string& line){
  std::vector<std::string> row;
  std::string cell;
  bool inQuotes=false;
  for(size_t i=0;i<line.length();++i){
    char c=line[i];
    if(inQuotes){
      if(c=='"'){
        if(i+1<line.length() && line[i+1]=='"'){ cell+='"'; ++i; }
        else inQuotes=false;
      }else cell+=c;
    }else{
      if(c=='"' && cell.empty()) inQuotes=true;
      else if(c==','){ row.push_back(cell); cell=""; }
      else cell+=c;
    }
  }
  row.push_back(cell);
  return row;
}

// Single character-by-character state machine over the whole input (RFC 4180).
// Quoted cells may span lines and contain commas and "" escapes, so the input
// is never pre-split on newlines.  Blank lines are skipped.
std::vector<std::vector<std::string>> parseCSV(const std::string& data){
  std::vector<std::vector<std::string>> result;
  std::vector<std::string> row;
  std::string cell;
  bool inQuotes=false;
  bool rowHasContent=false;
  for(size_t i=0;i<data.length();++i){
    char c=data[i];
    if(inQuotes){
      if(c=='"'){
        if(i+1<data.length() && data[i+1]=='"'){ cell+='"'; ++i; }
        else inQuotes=false;
      }else cell+=c;
    }else{
      if(c=='"' && cell.empty()){ inQuotes=true; rowHasContent=true; }
      else if(c==','){ row.push_back(cell); cell=""; rowHasContent=true; }
      else if(c=='\n' || (c=='\r' && i+1<data.length() && data[i+1]=='\n')){
        if(c=='\r') ++i;
        if(rowHasContent){ row.push_back(cell); result.push_back(row); }
        row.clear(); cell=""; rowHasContent=false;
      }
      else{ cell+=c; rowHasContent=true; }
    }
  }
  if(rowHasContent){ row.push_back(cell); result.push_back(row); }
  return result;
}
