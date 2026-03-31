#include "csv_serializer.h"

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
      const auto& cell = row[j];
      bool needsQuote = cell.find(',') != std::string::npos ||
                        cell.find('"') != std::string::npos ||
                        cell.find('\n') != std::string::npos;
      if(needsQuote) {
        result += "\"";
        for(char c : cell) {
          if(c == '"') result += "\"\"";
          else result += c;
        }
        result += "\"";
      } else {
        result += cell;
      }
    }
    result += "\r\n";
  }
  return result;
}
