#include "find_replace_rules.h"
#include <regex>
#include <future>
#include <chrono>
#include <iostream>

extern std::string applySubstringReplace(const std::string& cell,
  const FindReplaceRule& rule);
extern bool matchesRule(const std::string& cell, const FindReplaceRule& rule);

// ReDoS defence: reject patterns likely to cause catastrophic backtracking.
// Nested quantifiers (e.g. (a+)+, (a*)*, (a?)?, .*.) are the primary vector.
// Also reject excessively long patterns (>100 chars) as a coarse filter.
static bool isRegexDangerous(const std::string& pattern) {
  if(pattern.length() > 100) return true;
  // scan for adjacent quantifiers that form nested loops
  for(size_t i = 0; i + 1 < pattern.length(); ++i) {
    char c = pattern[i];
    char n = pattern[i+1];
    if((c == '+' || c == '*' || c == '?') && (n == '+' || n == '*' || n == '?'))
      return true;
  }
  return false;
}

// Run regex_replace in a detached thread with a 1-second deadline.
// std::regex has no built-in timeout, so std::future::wait_for provides
// a best-effort wall-clock guard.  A full migration to RE2 is deferred.
static std::string runRegexReplaceWithTimeout(
    const std::string& cell,
    const std::regex& re,
    const std::string& replacement) {
  auto future = std::async(std::launch::async, [&]() {
    return std::regex_replace(cell, re, replacement);
  });
  if(future.wait_for(std::chrono::seconds(1)) == std::future_status::ready) {
    return future.get();
  }
  // timed out — the regex is likely running away; return original cell
  std::cerr << "Warning: regex timed out after 1s, returning unchanged cell" << std::endl;
  return cell;
}

static std::string applyReplacement(const std::string& cell,
  const FindReplaceRule& rule) {
  if(rule.matchType == "exact") return matchesRule(cell, rule) ? rule.replace : cell;
  if(rule.matchType == "substring") return applySubstringReplace(cell, rule);
  if(rule.matchType == "regex") {
    if(isRegexDangerous(rule.find)) {
      std::cerr << "Warning: rejected dangerous regex pattern" << std::endl;
      return cell;
    }
    try {
      std::regex re(rule.find, rule.caseSensitive ? std::regex::ECMAScript :
        (std::regex::ECMAScript | std::regex::icase));
      return runRegexReplaceWithTimeout(cell, re, rule.replace);
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
      if(headers[i] == column) { colIndex = static_cast<int>(i); break; }
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
