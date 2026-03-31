#include "find_replace.h"
#include <regex>

extern std::string applySubstringReplace(const std::string& cell,
  const FindReplaceRule& rule);
extern bool matchesRule(const std::string& cell, const FindReplaceRule& rule);

static std::string applyReplacement(const std::string& cell,
  const FindReplaceRule& rule) {
  if(rule.matchType == "exact") return matchesRule(cell, rule) ? rule.replace : cell;
  if(rule.matchType == "substring") return applySubstringReplace(cell, rule);
  if(rule.matchType == "regex") {
    try {
      std::regex re(rule.find, rule.caseSensitive ? std::regex::ECMAScript :
        (std::regex::ECMAScript | std::regex::icase));
      return std::regex_replace(cell, re, rule.replace);
    } catch(...) { return cell; }
  }
  return cell;
}

FindReplaceResult applyFindReplace(const std::vector<std::vector<std::string>>& data,
  const std::string& column, const std::vector<FindReplaceRule>& rules,
  const std::vector<std::string>& headers) {
  FindReplaceResult result;
  result.totalReplacements = 0;
  int colIndex = -1;
  if(column != "*") {
    for(size_t i = 0; i < headers.size(); i++) {
      if(headers[i] == column) { colIndex = i; break; }
    }
    if(colIndex == -1) { result.data = data; return result; }
  }
  result.data = data;
  for(auto& row : result.data) {
    if(column == "*") {
      for(size_t j = 0; j < row.size(); j++) {
        std::string orig = row[j];
        for(const auto& rule : rules) row[j] = applyReplacement(row[j], rule);
        if(row[j] != orig) result.totalReplacements++, result.replacementCounts[headers[j]]++;
      }
    } else if(colIndex >= 0 && colIndex < (int)row.size()) {
      std::string orig = row[colIndex];
      for(const auto& rule : rules) row[colIndex] = applyReplacement(row[colIndex], rule);
      if(row[colIndex] != orig) result.totalReplacements++, result.replacementCounts[column]++;
    }
  }
  return result;
}
