#include "find_replace_rules.h"
#include <algorithm>
#include <cctype>

static std::string lowerCaseStr(const std::string& s) {
  std::string r = s;
  std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  return r;
}

std::string applySubstringReplace(const std::string& cell,
  const FindReplaceRule& rule) {
  std::string find = rule.caseSensitive ? rule.find : lowerCaseStr(rule.find);
  std::string lower = lowerCaseStr(cell);
  if(!rule.caseSensitive) {
    std::string res;
    size_t last = 0, pos = 0;
    while((pos = lower.find(find, pos)) != std::string::npos) {
      res += cell.substr(last, pos - last) + rule.replace;
      last = pos + find.length();
      pos += find.length();
    }
    return res + cell.substr(last);
  }
  std::string cellCopy = cell;
  for(size_t pos = 0; (pos = cellCopy.find(rule.find, pos)) != std::string::npos;) {
    cellCopy.replace(pos, rule.find.length(), rule.replace);
    pos += rule.replace.length();
  }
  return cellCopy;
}
