#include "deep_clean.h"
#include "column_type_detection.h"
#include "weighted_dedup.h"
#include "structural_cleaners.h"
#include "string_issue_detectors.h"
#include "text_normalisation.h"
#include "cluster_detection.h"

#include <algorithm>

// --- helpers ------------------------------------------------------------

static int countChangedCells(const std::vector<std::vector<std::string>>& before,
                             const std::vector<std::vector<std::string>>& after) {
  int changed = 0;
  size_t rows = std::min(before.size(), after.size());
  for (size_t r = 0; r < rows; r++) {
    size_t cols = std::min(before[r].size(), after[r].size());
    for (size_t c = 0; c < cols; c++) {
      if (before[r][c] != after[r][c]) changed++;
    }
  }
  return changed;
}

// --- per-type cell transform --------------------------------------------

static std::string transformCell(const std::string& cell, ColumnType type) {
  if (cell.empty()) return "";

  switch (type) {
    case ColumnType::EMAIL:
      return toLowerCase(cell);

    case ColumnType::URL:
      return toLowerCase(cell);

    case ColumnType::PHONE:
      {
        std::string out;
        for (char c : cell) {
          if (c == '+' && out.empty()) out += c;
          else if (c >= '0' && c <= '9') out += c;
        }
        // fall back to original if stripping removes everything
        return out.empty() ? cell : out;
      }

    case ColumnType::DATE:
      {
        // normalize to ISO YYYY-MM-DD
        if (cell.size() == 10 && cell[4] == '-' && cell[7] == '-') return cell;
        if (cell.size() == 10 && cell[4] == '/' && cell[7] == '/') {
          return cell.substr(0,4) + "-" + cell.substr(5,2) + "-" + cell.substr(8,2);
        }
        if ((cell.size() == 10 && cell[2] == '/' && cell[5] == '/') ||
            (cell.size() == 10 && cell[2] == '-' && cell[5] == '-')) {
          return cell.substr(6,4) + "-" + cell.substr(3,2) + "-" + cell.substr(0,2);
        }
        return cell; // unknown format — leave as-is
      }

    case ColumnType::NAME:
      return toUpperCase(cell);

    case ColumnType::GENERIC_TEXT:
      return toUpperCase(cell);

    case ColumnType::FREE_TEXT:
      return cell;  // no case change

    case ColumnType::NUMERIC:
      {
        // strip commas, spaces, and leading currency symbols
        std::string out;
        bool started = false;
        for (size_t i = 0; i < cell.size(); i++) {
          char c = cell[i];
          if (c == ',' || c == ' ') continue;
          // leading currency symbol: $ or multi-byte UTF-8 £ (0xC2 0xA3) / € (0xE2 0x82 0xAC)
          if (!started) {
            if (c == '$') continue;
            if (c == '\xC2' && i + 1 < cell.size() && cell[i + 1] == '\xA3') { i++; continue; }
            if (c == '\xE2' && i + 2 < cell.size() && cell[i + 1] == '\x82' && cell[i + 2] == '\xAC') { i += 2; continue; }
          }
          out += c;
          started = true;
        }
        return out.empty() ? cell : out;
      }

    case ColumnType::BOOLEAN:
      {
        std::string lower = toLowerCase(cell);
        if (lower == "true" || lower == "yes" || lower == "1" || lower == "t" || lower == "y") return "true";
        if (lower == "false" || lower == "no" || lower == "0" || lower == "f" || lower == "n") return "false";
        return cell; // not a recognizable boolean
      }

    case ColumnType::ID:
      {
        size_t s = cell.find_first_not_of(" \t\r\n");
        size_t e = cell.find_last_not_of(" \t\r\n");
        if (s == std::string::npos) return "";
        return cell.substr(s, e - s + 1);
      }

    default:
      return cell;
  }
}

// --- per-type transform over entire column ------------------------------

static std::vector<std::vector<std::string>> applyTransforms(
    const std::vector<std::vector<std::string>>& data,
    const std::vector<ColumnType>& columnTypes,
    AuditLog& auditLog,
    const std::vector<std::string>& columnNames) {

  if (data.empty()) return data;

  std::vector<std::vector<std::string>> result = data;
  size_t nCols = std::min(columnTypes.size(), data[0].size());

  for (size_t col = 0; col < nCols; col++) {
    int changed = 0;
    for (size_t row = 0; row < data.size(); row++) {
      if (col < data[row].size()) {
        std::string transformed = transformCell(data[row][col], columnTypes[col]);
        if (data[row][col] != transformed) changed++;
        result[row][col] = transformed;
      }
    }
    if (changed > 0) {
      std::string opName = std::string("Standardise Column: ") + columnNames[col] +
                           " (" + columnTypeToString(columnTypes[col]) + ")";
      auditLog.addEntry(opName, changed, (int)data.size(), (int)data.size(), "standardise");
    }
  }
  return result;
}

// --- auto-merge pre-seeding ---------------------------------------------

static std::vector<std::vector<std::string>> autoMergeColumns(
    const std::vector<std::vector<std::string>>& data,
    const std::vector<ColumnType>& columnTypes,
    const std::vector<std::string>& columnNames,
    AuditLog& auditLog) {

  if (data.size() <= 1) return data;

  std::vector<std::vector<std::string>> result = data;
  const auto& headers = data[0];
  int totalMerges = 0;

  for (size_t col = 0; col < columnTypes.size() && col < headers.size(); col++) {
    // only auto-merge text-type columns
    if (columnTypes[col] != ColumnType::NAME &&
        columnTypes[col] != ColumnType::GENERIC_TEXT &&
        columnTypes[col] != ColumnType::FREE_TEXT)
      continue;

    // detect clusters at high threshold
    ClusterResult clusters = detectClusters(result, headers[col], 0.95, headers);

    // build merge mappings: merge all cluster members into the most frequent value
    std::vector<MergeMapping> merges;
    for (const auto& cluster : clusters.clusters) {
      if (cluster.values.size() <= 1) continue;
      // pick the shortest value as canonical (or first — deterministic)
      std::string canonical = cluster.values[0];
      for (const auto& v : cluster.values) {
        if (v.size() < canonical.size()) canonical = v;
      }
      std::vector<std::string> others;
      for (const auto& v : cluster.values) {
        if (v != canonical) others.push_back(v);
      }
      if (!others.empty()) {
        MergeMapping mm;
        mm.clusterId = cluster.id;
        mm.mergeInto = canonical;
        mm.values = others;
        merges.push_back(mm);
      }
    }

    if (!merges.empty()) {
      int before = (int)result.size();
      result = applyClustering(result, headers[col], merges, headers);
      totalMerges += (int)merges.size();
      auditLog.addEntry(
          "Auto-merge Column: " + columnNames[col] + " (" + std::to_string(merges.size()) + " groups)",
          0, before, (int)result.size(), "merge");
    }
  }

  return result;
}

// --- main pipeline ------------------------------------------------------

DeepCleanResult deepClean(const std::vector<std::vector<std::string>>& parsed) {
  DeepCleanResult result;
  if (parsed.empty()) return result;

  int originalRows = (int)parsed.size();

  // Phase 1: Tidy — trim + collapse whitespace, no case change
  auto tidied = trimWhitespace(parsed);
  // also strip cells that are purely whitespace
  for (auto& row : tidied) {
    for (auto& cell : row) {
      bool allWs = true;
      for (char c : cell) { if (c != ' ' && c != '\t' && c != '\r' && c != '\n') { allWs = false; break; } }
      if (allWs) cell.clear();
    }
  }
  result.auditLog.addEntry("Tidy Whitespace", countChangedCells(parsed, tidied),
                           originalRows, (int)tidied.size(), "tidy");

  // Phase 2: Standardise nulls
  auto nulled = standardiseNullValuesInData(tidied);
  result.auditLog.addEntry("Standardise Null Values", countChangedCells(tidied, nulled),
                           (int)tidied.size(), (int)nulled.size(), "nulls");

  // Phase 3: Detect column types
  auto typeResult = detectColumnTypes(nulled);
  result.columnTypes = typeResult.types;
  result.columnNames = typeResult.names;
  {
    std::string details;
    for (size_t i = 0; i < typeResult.types.size(); i++) {
      if (i > 0) details += ", ";
      details += std::string(typeResult.names[i]) + ":" + columnTypeToString(typeResult.types[i]);
    }
    result.auditLog.addEntry("Detect Column Types [" + details + "]", 0,
                             (int)nulled.size(), (int)nulled.size(), "detect-types");
  }

  // Phase 4: Per-type transforms
  auto transformed = applyTransforms(nulled, result.columnTypes, result.auditLog, result.columnNames);

  // Phase 5: Auto-merge pre-seeding (optional — only for text columns at high threshold)
  auto merged = autoMergeColumns(transformed, result.columnTypes, result.columnNames, result.auditLog);

  // Phase 6: Exact dedup
  auto exactDeduped = removeDuplicates(merged);
  int exactRemoved = (int)merged.size() - (int)exactDeduped.size();
  result.auditLog.addEntry("Exact Deduplication", 0, (int)merged.size(), (int)exactDeduped.size(),
                           "dedup-pass-1-exact");

  // Phase 7: Weighted fuzzy dedup
  auto fuzzyDeduped = weightedDeduplicate(exactDeduped, result.columnTypes, 0.95);
  result.auditLog.addEntry("Weighted Fuzzy Deduplication", 0,
                           (int)exactDeduped.size(), (int)fuzzyDeduped.data.size(),
                           "dedup-pass-1-fuzzy");

  result.cleanedData = fuzzyDeduped.data;
  return result;
}
