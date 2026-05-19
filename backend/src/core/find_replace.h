#ifndef FIND_REPLACE_H
#define FIND_REPLACE_H
#include <vector>
#include <string>
#include <map>

struct FindReplaceRule {
  std::string find;
  std::string replace;
  std::string matchType;
  bool caseSensitive;
};

struct FindReplaceResult {
  std::vector<std::vector<std::string>> data;
  std::map<std::string, int> replacementCounts;
  int totalReplacements;
};

bool matchesRule(const std::string& cell, const FindReplaceRule& rule);
std::string applySubstringReplace(const std::string& cell,
  const FindReplaceRule& rule);
FindReplaceResult applyFindReplace(
  const std::vector<std::vector<std::string>>& data,
  const std::string& column,
  const std::vector<FindReplaceRule>& rules,
  const std::vector<std::string>& headers);

#endif
