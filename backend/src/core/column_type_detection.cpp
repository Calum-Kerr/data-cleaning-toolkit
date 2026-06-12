#include "column_type_detection.h"
#include "text_normalisation.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <map>

// --- header hint map ----------------------------------------------------

// Split a header name into words delimited by underscore or space, then
// match each word exactly (or by prefix for compound hints like "updated_at"
// where "updated" alone should signal DATE).  This avoids false positives
// from substring matches: "tel" no longer matches "hotel", "id" no longer
// matches "holiday" or "paid", and "is_"/"has_" now match before underscores
// are stripped.
static bool wordMatches(const std::string& word, const std::string& candidate) {
  return word == candidate;
}

static bool hasWord(const std::vector<std::string>& words, const std::string& candidate) {
  for (const auto& w : words) if (w == candidate) return true;
  return false;
}

static ColumnType hintFromHeader(const std::string& header) {
  std::string h = toLowerCase(header);

  // split into words by underscore or space
  std::vector<std::string> words;
  std::string current;
  for (char c : h) {
    if (c == '_' || c == ' ') {
      if (!current.empty()) { words.push_back(current); current.clear(); }
    } else {
      current += c;
    }
  }
  if (!current.empty()) words.push_back(current);

  if (hasWord(words, "email") || hasWord(words, "mail")) return ColumnType::EMAIL;
  if (hasWord(words, "phone") || hasWord(words, "mobile") ||
      hasWord(words, "tel") || hasWord(words, "telephone") ||
      hasWord(words, "fax")) return ColumnType::PHONE;
  if (hasWord(words, "url") || hasWord(words, "website") ||
      hasWord(words, "web") || hasWord(words, "link")) return ColumnType::URL;
  if (hasWord(words, "date") || hasWord(words, "dob") ||
      hasWord(words, "birth") || hasWord(words, "timestamp") ||
      hasWord(words, "time") || hasWord(words, "created") ||
      hasWord(words, "updated") || hasWord(words, "createdat") ||
      hasWord(words, "updatedat")) return ColumnType::DATE;
  if (hasWord(words, "comment") || hasWord(words, "description") ||
      hasWord(words, "note") || hasWord(words, "message") ||
      hasWord(words, "review") || hasWord(words, "text") ||
      hasWord(words, "bio")) return ColumnType::FREE_TEXT;
  if (hasWord(words, "id") || hasWord(words, "code") ||
      hasWord(words, "key") || hasWord(words, "uuid") ||
      hasWord(words, "guid") || hasWord(words, "ref")) return ColumnType::ID;
  if (hasWord(words, "name") || hasWord(words, "first") ||
      hasWord(words, "last") || hasWord(words, "full") ||
      hasWord(words, "city") || hasWord(words, "state") ||
      hasWord(words, "country") || hasWord(words, "street") ||
      hasWord(words, "address")) return ColumnType::NAME;
  if (hasWord(words, "price") || hasWord(words, "cost") ||
      hasWord(words, "amount") || hasWord(words, "salary") ||
      hasWord(words, "age") || hasWord(words, "count") ||
      hasWord(words, "number") || hasWord(words, "score") ||
      hasWord(words, "rate") || hasWord(words, "fee")) return ColumnType::NUMERIC;
  if (hasWord(words, "bool") || hasWord(words, "flag") ||
      hasWord(words, "active") || hasWord(words, "enabled") ||
      hasWord(words, "is") || hasWord(words, "has")) return ColumnType::BOOLEAN;

  return ColumnType::GENERIC_TEXT;  // no hint matched
}

// --- validators ---------------------------------------------------------

static bool isValidEmail(const std::string& s) {
  // must contain exactly one @, with at least one char before and after,
  // a dot somewhere after the @, and no whitespace
  if (s.find(' ') != std::string::npos) return false;
  size_t at = s.find('@');
  if (at == std::string::npos || at == 0 || at == s.size() - 1) return false;
  std::string domain = s.substr(at + 1);
  if (domain.find('.') == std::string::npos) return false;
  return true;
}

static bool isValidURL(const std::string& s) {
  std::string lower = toLowerCase(s);
  if (lower.find("http://") == 0 || lower.find("https://") == 0) return true;
  if (lower.find("www.") == 0) return true;
  // common TLD pattern
  if (lower.find(".com") != std::string::npos ||
      lower.find(".org") != std::string::npos ||
      lower.find(".net") != std::string::npos ||
      lower.find(".io")  != std::string::npos) {
    return lower.find(' ') == std::string::npos;
  }
  return false;
}

static bool isValidPhone(const std::string& s) {
  // count digits after stripping common separators
  int digits = 0;
  for (char c : s) {
    if (c >= '0' && c <= '9') digits++;
    else if (c != ' ' && c != '-' && c != '.' && c != '(' && c != ')' && c != '+')
      return false; // unexpected character
  }
  return digits >= 7;
}

static bool isValidDate(const std::string& s) {
  // ISO: YYYY-MM-DD or YYYY/MM/DD
  if (s.size() == 10 && s[4] == '-' && s[7] == '-') {
    int y = 0, m = 0, d = 0;
    try { y = std::stoi(s.substr(0, 4)); m = std::stoi(s.substr(5, 2)); d = std::stoi(s.substr(8, 2)); }
    catch (...) { return false; }
    return y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31;
  }
  if (s.size() == 10 && s[4] == '/' && s[7] == '/') {
    int y = 0, m = 0, d = 0;
    try { y = std::stoi(s.substr(0, 4)); m = std::stoi(s.substr(5, 2)); d = std::stoi(s.substr(8, 2)); }
    catch (...) { return false; }
    return y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31;
  }
  // DD/MM/YYYY
  if (s.size() == 10 && s[2] == '/' && s[5] == '/') {
    int d = 0, m = 0, y = 0;
    try { d = std::stoi(s.substr(0, 2)); m = std::stoi(s.substr(3, 2)); y = std::stoi(s.substr(6, 4)); }
    catch (...) { return false; }
    return y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31;
  }
  // DD-MM-YYYY
  if (s.size() == 10 && s[2] == '-' && s[5] == '-') {
    int d = 0, m = 0, y = 0;
    try { d = std::stoi(s.substr(0, 2)); m = std::stoi(s.substr(3, 2)); y = std::stoi(s.substr(6, 4)); }
    catch (...) { return false; }
    return y >= 1900 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31;
  }
  return false;
}

static bool isValidNumeric(const std::string& s) {
  if (s.empty()) return false;
  std::string trimmed;
  for (char c : s) {
    if (c != ' ' && c != ',') trimmed += c;  // allow commas as thousand separators
  }
  try {
    size_t pos = 0;
    std::stod(trimmed, &pos);
    return pos == trimmed.size();  // whole string consumed
  } catch (...) {
    return false;
  }
}

static bool isValidBoolean(const std::string& s) {
  std::string lower = toLowerCase(s);
  return lower == "true" || lower == "false" ||
         lower == "yes" || lower == "no" ||
         lower == "1" || lower == "0" ||
         lower == "t" || lower == "f" ||
         lower == "y" || lower == "n";
}

static bool isValidID(const std::string& s) {
  if (s.empty()) return false;
  // alphanumeric with optional dashes/underscores, 3-50 chars
  if (s.size() < 3 || s.size() > 50) return false;
  for (char c : s) {
    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_'))
      return false;
  }
  return true;
}

static bool isValidName(const std::string& s) {
  if (s.empty()) return false;
  // mostly alphabetic and spaces, with occasional hyphens/apostrophes
  int alpha = 0, other = 0;
  for (char c : s) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ') alpha++;
    else if (c == '-' || c == '\'' || c == '.') other++;
    else return false;
  }
  return alpha > 0 && (alpha + other) == (int)s.size();
}

static bool looksLikeFreeText(const std::string& s) {
  // longer strings, or strings with sentence punctuation
  if (s.size() > 50) return true;
  for (char c : s) {
    if (c == '.' || c == '!' || c == '?' || c == ',' || c == ';' || c == ':')
      return true;
  }
  return false;
}

// --- scoring ------------------------------------------------------------

static const int SAMPLE_SIZE = 200;

static std::vector<std::string> sampleColumn(const std::vector<std::vector<std::string>>& data,
                                             size_t colIdx) {
  std::vector<std::string> sample;
  for (size_t row = 1; row < data.size() && sample.size() < (size_t)SAMPLE_SIZE; row++) {
    if (colIdx < data[row].size() && !data[row][colIdx].empty()) {
      sample.push_back(data[row][colIdx]);
    }
  }
  return sample;
}

struct TypeScore {
  ColumnType type;
  double score;
};

static ColumnType detectOneColumn(const std::vector<std::vector<std::string>>& data,
                                  size_t colIdx,
                                  const std::string& headerName) {
  std::vector<std::string> sample = sampleColumn(data, colIdx);
  if (sample.empty()) {
    // no non-empty data — fall back to header hint
    return hintFromHeader(headerName);
  }

  // score each type
  std::vector<TypeScore> scores;

  auto scoreType = [&](ColumnType t, auto validator) -> double {
    if (sample.empty()) return 0.0;
    int matches = 0;
    for (const auto& v : sample) {
      if (validator(v)) matches++;
    }
    return (double)matches / (double)sample.size();
  };

  // evaluate in deterministic precedence order
  scores.push_back({ColumnType::EMAIL,       scoreType(ColumnType::EMAIL, isValidEmail)});
  scores.push_back({ColumnType::PHONE,       scoreType(ColumnType::PHONE, isValidPhone)});
  scores.push_back({ColumnType::URL,         scoreType(ColumnType::URL, isValidURL)});
  scores.push_back({ColumnType::DATE,        scoreType(ColumnType::DATE, isValidDate)});
  scores.push_back({ColumnType::NUMERIC,     scoreType(ColumnType::NUMERIC, isValidNumeric)});
  scores.push_back({ColumnType::BOOLEAN,     scoreType(ColumnType::BOOLEAN, isValidBoolean)});
  scores.push_back({ColumnType::ID,          scoreType(ColumnType::ID, isValidID)});
  scores.push_back({ColumnType::NAME,        scoreType(ColumnType::NAME, isValidName)});

  // FREE_TEXT: average length > 50 or contains sentence punctuation
  {
    int freeTextMatches = 0;
    for (const auto& v : sample) {
      if (looksLikeFreeText(v)) freeTextMatches++;
    }
    double avgLen = 0;
    for (const auto& v : sample) avgLen += v.size();
    avgLen /= sample.size();
    double ftScore = std::max((double)freeTextMatches / sample.size(),
                              avgLen > 50 ? 0.7 : 0.0);
    scores.push_back({ColumnType::FREE_TEXT, ftScore});
  }

  // GENERIC_TEXT: default fallback — always score 1.0
  scores.push_back({ColumnType::GENERIC_TEXT, 1.0});

  // find best match
  ColumnType bestType = ColumnType::GENERIC_TEXT;
  double bestScore = 0.0;
  ColumnType hintType = hintFromHeader(headerName);

  for (const auto& s : scores) {
    // if the header hint matches this type, boost the score
    double adjustedScore = s.score;
    if (s.type == hintType && s.type != ColumnType::GENERIC_TEXT) {
      adjustedScore = std::min(1.0, s.score + 0.15);
    }
    if (adjustedScore >= 0.70 && adjustedScore > bestScore) {
      bestScore = adjustedScore;
      bestType = s.type;
    }
  }

  // if nothing reached 70%, use the header hint if available
  if (bestScore < 0.70 && hintType != ColumnType::GENERIC_TEXT) {
    return hintType;
  }
  if (bestScore < 0.70) {
    return ColumnType::GENERIC_TEXT;
  }
  return bestType;
}

// --- public API ---------------------------------------------------------

ColumnTypeResult detectColumnTypes(const std::vector<std::vector<std::string>>& data) {
  ColumnTypeResult result;
  if (data.empty() || data[0].empty()) return result;

  const auto& headers = data[0];
  result.types.resize(headers.size(), ColumnType::GENERIC_TEXT);
  result.names = headers;

  for (size_t col = 0; col < headers.size(); col++) {
    result.types[col] = detectOneColumn(data, col, headers[col]);
  }
  return result;
}

const char* columnTypeToString(ColumnType t) {
  switch (t) {
    case ColumnType::EMAIL:         return "EMAIL";
    case ColumnType::PHONE:         return "PHONE";
    case ColumnType::URL:           return "URL";
    case ColumnType::DATE:          return "DATE";
    case ColumnType::NUMERIC:       return "NUMERIC";
    case ColumnType::BOOLEAN:       return "BOOLEAN";
    case ColumnType::ID:            return "ID";
    case ColumnType::NAME:          return "NAME";
    case ColumnType::FREE_TEXT:     return "FREE_TEXT";
    case ColumnType::GENERIC_TEXT:  return "GENERIC_TEXT";
    default: return "UNKNOWN";
  }
}

double typeWeight(ColumnType t) {
  switch (t) {
    case ColumnType::EMAIL:         return 3.0;
    case ColumnType::PHONE:         return 3.0;
    case ColumnType::ID:            return 3.0;
    case ColumnType::NAME:          return 2.0;
    case ColumnType::DATE:          return 1.5;
    case ColumnType::URL:           return 1.5;
    case ColumnType::NUMERIC:       return 1.0;
    case ColumnType::BOOLEAN:       return 0.5;
    case ColumnType::GENERIC_TEXT:  return 1.0;
    case ColumnType::FREE_TEXT:     return 0.5;
    default: return 1.0;
  }
}
