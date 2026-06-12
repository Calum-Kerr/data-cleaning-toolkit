#include "weighted_dedup.h"
#include "string_issue_detectors.h"
#include "text_normalisation.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_set>

static const int BLOCKING_WINDOW = 20;

// --- cell normalisation per type ----------------------------------------

static std::string normaliseForType(const std::string& cell, ColumnType type) {
  switch (type) {
    case ColumnType::EMAIL:
    case ColumnType::URL:
      return toLowerCase(cell);

    case ColumnType::PHONE:
      {
        // strip everything except digits and leading +
        std::string out;
        for (char c : cell) {
          if (c == '+' && out.empty()) out += c;
          else if (c >= '0' && c <= '9') out += c;
        }
        return out;
      }

    case ColumnType::DATE:
      // normalize to ISO-ish for comparison — try YYYY-MM-DD first
      if (cell.size() == 10 && cell[4] == '-' && cell[7] == '-') return cell;
      if (cell.size() == 10 && cell[4] == '/' && cell[7] == '/') {
        return cell.substr(0,4) + "-" + cell.substr(5,2) + "-" + cell.substr(8,2);
      }
      // DD/MM/YYYY or DD-MM-YYYY → YYYY-MM-DD
      if ((cell.size() == 10 && cell[2] == '/' && cell[5] == '/') ||
          (cell.size() == 10 && cell[2] == '-' && cell[5] == '-')) {
        return cell.substr(6,4) + "-" + cell.substr(3,2) + "-" + cell.substr(0,2);
      }
      return cell;

    case ColumnType::BOOLEAN:
      {
        std::string lower = toLowerCase(cell);
        if (lower == "true" || lower == "yes" || lower == "1" || lower == "t" || lower == "y") return "true";
        if (lower == "false" || lower == "no" || lower == "0" || lower == "f" || lower == "n") return "false";
        return lower;
      }

    case ColumnType::NUMERIC:
      {
        // strip commas, spaces, and leading currency symbols
        std::string out;
        bool started = false;
        for (char c : cell) {
          if (c == ',' || c == ' ') continue;
          if (!started && (c == '$' || c == '£' || c == '€')) continue;
          out += c;
          started = true;
        }
        try {
          double v = std::stod(out);
          return std::to_string(v);
        } catch (...) {
          return out;
        }
      }

    case ColumnType::ID:
      // trim and preserve exact value
      {
        size_t s = cell.find_first_not_of(" \t\r\n");
        size_t e = cell.find_last_not_of(" \t\r\n");
        if (s == std::string::npos) return "";
        return cell.substr(s, e - s + 1);
      }

    case ColumnType::NAME:
    case ColumnType::GENERIC_TEXT:
    case ColumnType::FREE_TEXT:
    default:
      return cell;
  }
}

// --- cell similarity by type --------------------------------------------

static double cellSimilarity(const std::string& a, const std::string& b, ColumnType type) {
  // missing-value half-credit
  if (a.empty() || b.empty()) return 0.5;

  switch (type) {
    case ColumnType::EMAIL:
    case ColumnType::PHONE:
    case ColumnType::ID:
      {
        std::string na = normaliseForType(a, type);
        std::string nb = normaliseForType(b, type);
        return na == nb ? 1.0 : 0.0;
      }

    case ColumnType::URL:
    case ColumnType::DATE:
    case ColumnType::NUMERIC:
    case ColumnType::BOOLEAN:
      {
        std::string na = normaliseForType(a, type);
        std::string nb = normaliseForType(b, type);
        return na == nb ? 1.0 : 0.0;
      }

    case ColumnType::NAME:
    case ColumnType::GENERIC_TEXT:
    case ColumnType::FREE_TEXT:
      return calculateSimilarity(a, b);  // Levenshtein-based

    default:
      return a == b ? 1.0 : 0.0;
  }
}

// --- row similarity (weighted) ------------------------------------------

static double rowSimilarity(const std::vector<std::string>& r1,
                            const std::vector<std::string>& r2,
                            const std::vector<ColumnType>& columnTypes) {
  size_t minCols = std::min({r1.size(), r2.size(), columnTypes.size()});
  if (minCols == 0) return 0.0;

  double weightedSum = 0.0;
  double totalWeight = 0.0;
  bool idVeto = false;

  for (size_t i = 0; i < minCols; i++) {
    double w = typeWeight(columnTypes[i]);
    double sim = cellSimilarity(r1[i], r2[i], columnTypes[i]);

    // hard veto: identifier disagreement kills the row
    if (columnTypes[i] == ColumnType::ID && !r1[i].empty() && !r2[i].empty()) {
      std::string n1 = normaliseForType(r1[i], ColumnType::ID);
      std::string n2 = normaliseForType(r2[i], ColumnType::ID);
      if (n1 != n2) {
        idVeto = true;
      }
    }

    weightedSum += w * sim;
    totalWeight += w;
  }

  if (idVeto) return 0.0;
  if (totalWeight == 0.0) return 0.0;
  return weightedSum / totalWeight;
}

// --- blocking key -------------------------------------------------------

static std::string blockingKey(const std::vector<std::string>& row,
                               const std::vector<ColumnType>& columnTypes) {
  std::string key;
  size_t n = std::min(row.size(), columnTypes.size());
  for (size_t i = 0; i < n; i++) {
    // use non-ID text columns for the blocking key
    if (columnTypes[i] == ColumnType::ID ||
        columnTypes[i] == ColumnType::NUMERIC ||
        columnTypes[i] == ColumnType::BOOLEAN ||
        columnTypes[i] == ColumnType::DATE) continue;

    std::string norm = normaliseForType(row[i], columnTypes[i]);
    // lower-case and strip spaces for a fuzzy sort key
    std::string flat;
    for (char c : norm) {
      if (c >= 'A' && c <= 'Z') flat += (char)(c + 32);
      else if (c != ' ' && c != '\t' && c != '\r' && c != '\n') flat += c;
    }
    if (!flat.empty()) {
      if (!key.empty()) key += '|';
      key += flat;
    }
  }
  return key;
}

// --- main dedup function ------------------------------------------------

WeightedDedupResult weightedDeduplicate(
    const std::vector<std::vector<std::string>>& data,
    const std::vector<ColumnType>& columnTypes,
    double threshold) {

  WeightedDedupResult result;
  if (data.size() <= 1) {
    result.data = data;
    result.rowsRemoved = 0;
    return result;
  }

  // build (blockingKey, originalIndex) pairs for data rows only (skip header)
  struct IndexedKey {
    std::string key;
    size_t originalIdx;
  };
  std::vector<IndexedKey> indexed;
  indexed.reserve(data.size() - 1);
  for (size_t i = 1; i < data.size(); i++) {
    indexed.push_back({blockingKey(data[i], columnTypes), i});
  }

  // stable sort by blocking key
  std::stable_sort(indexed.begin(), indexed.end(),
                   [](const IndexedKey& a, const IndexedKey& b) {
                     return a.key < b.key;
                   });

  // sliding-window dedup
  std::vector<bool> isDuplicate(data.size(), false);
  isDuplicate[0] = false; // header is never a duplicate

  for (size_t i = 0; i < indexed.size(); i++) {
    size_t origI = indexed[i].originalIdx;
    if (isDuplicate[origI]) continue;

    size_t windowEnd = std::min(indexed.size(), i + BLOCKING_WINDOW);
    for (size_t j = i + 1; j < windowEnd; j++) {
      size_t origJ = indexed[j].originalIdx;
      if (isDuplicate[origJ]) continue;

      double sim = rowSimilarity(data[origI], data[origJ], columnTypes);
      if (sim >= threshold) {
        isDuplicate[origJ] = true;
      }
    }
  }

  // build result preserving original order
  result.data.push_back(data[0]); // header
  for (size_t i = 1; i < data.size(); i++) {
    if (!isDuplicate[i]) {
      result.data.push_back(data[i]);
    }
  }

  result.rowsRemoved = (int)data.size() - (int)result.data.size();
  return result;
}
