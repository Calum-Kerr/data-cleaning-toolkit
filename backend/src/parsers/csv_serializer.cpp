#include "csv_serializer.h"

// CSV formula injection defence: prepend a single quote to cells that start
// with a formula trigger character (= + - @) so spreadsheet applications
// (Excel, LibreOffice Calc) treat them as text, not executable formulas.
static std::string sanitizeFormulaCell(const std::string& cell) {
  if(!cell.empty()) {
    char first = cell[0];
    if(first == '=' || first == '+' || first == '-' || first == '@')
      return "'" + cell;
  }
  return cell;
}

std::string serializeToCSV(const std::vector<std::vector<std::string>>& data) {
  std::string result;
  for(const auto& row : data) {
    bool isEmptyRow = true;
    for(const auto& cell : row) {
      if(!cell.empty()) {
        isEmptyRow = false;
        break;
      }
    }
    if(isEmptyRow) continue;

    for(size_t j = 0; j < row.size(); j++) {
      if(j > 0) result += ",";
      std::string safe = sanitizeFormulaCell(row[j]);
      bool needsQuote = safe.find(',') != std::string::npos ||
                        safe.find('"') != std::string::npos ||
                        safe.find('\n') != std::string::npos;
      if(needsQuote) {
        result += "\"";
        for(char c : safe) {
          if(c == '"') result += "\"\"";
          else result += c;
        }
        result += "\"";
      } else {
        result += safe;
      }
    }
    result += "\r\n";
  }
  return result;
}
