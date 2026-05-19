#include "find_replace_rules.h"
#include <algorithm>
#include <cctype>

static std::string toLowerCase(const std::string& s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(),
    [](unsigned char c) { return std::tolower(c); });
  return result;
}

bool matchesRule(const std::string& cell, const FindReplaceRule& rule) {
  std::string cellToMatch = rule.caseSensitive ? cell : toLowerCase(cell);
  std::string findToMatch = rule.caseSensitive ? rule.find : toLowerCase(rule.find);
  if(rule.matchType == "exact") {
    return cellToMatch == findToMatch;
  } else if(rule.matchType == "substring") {
    return cellToMatch.find(findToMatch) != std::string::npos;
  }
  return false;
}

