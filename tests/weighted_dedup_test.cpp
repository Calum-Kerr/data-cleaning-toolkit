#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "column_type_detection.h"
#include "weighted_dedup.h"

class WeightedDedupTest {
public:
  std::vector<std::vector<std::string>> makeRows(
      const std::vector<std::string>& header,
      const std::vector<std::vector<std::string>>& dataRows) {
    std::vector<std::vector<std::string>> data;
    data.push_back(header);
    for (const auto& row : dataRows) data.push_back(row);
    return data;
  }

  void test_header_preserved() {
    auto data = makeRows(
      {"col1"},
      {{"foo"}, {"foo"}, {"foo"}}
    );
    std::vector<ColumnType> types = {ColumnType::GENERIC_TEXT};
    auto result = weightedDeduplicate(data, types, 0.95);
    // header must survive
    assert(result.data.size() >= 1);
    assert(result.data[0][0] == "col1");
    std::cout << "PASS: header preserved\n";
  }

  void test_exact_duplicate_removed() {
    auto data = makeRows(
      {"name"},
      {{"Alice"}, {"Bob"}, {"Alice"}}  // second Alice is duplicate
    );
    std::vector<ColumnType> types = {ColumnType::NAME};
    auto result = weightedDeduplicate(data, types, 0.95);
    // Should have 3 rows: header + Alice + Bob (second Alice removed)
    assert(result.data.size() == 3);
    assert(result.rowsRemoved == 1);
    std::cout << "PASS: exact duplicate removed\n";
  }

  void test_different_rows_kept() {
    auto data = makeRows(
      {"name"},
      {{"Alice"}, {"Bob"}, {"Charlie"}}
    );
    std::vector<ColumnType> types = {ColumnType::NAME};
    auto result = weightedDeduplicate(data, types, 0.95);
    // All three should be kept (all different)
    assert(result.data.size() == 4); // header + 3
    assert(result.rowsRemoved == 0);
    std::cout << "PASS: different rows kept\n";
  }

  void test_near_duplicate_above_threshold_removed() {
    auto data = makeRows(
      {"city"},
      {{"London"}, {"Londn"}, {"Paris"}}  // "Londn" is near "London"
    );
    std::vector<ColumnType> types = {ColumnType::NAME};
    auto result = weightedDeduplicate(data, types, 0.85);
    // "Londn" should be removed as near-duplicate of "London"
    assert(result.data.size() <= 3);
    assert(result.rowsRemoved >= 1);
    std::cout << "PASS: near-duplicate above threshold removed\n";
  }

  void test_below_threshold_kept() {
    auto data = makeRows(
      {"city"},
      {{"London"}, {"Manchester"}, {"Birmingham"}}
    );
    std::vector<ColumnType> types = {ColumnType::NAME};
    auto result = weightedDeduplicate(data, types, 0.99); // very high threshold
    // None should be removed — all very different
    assert(result.data.size() == 4);
    std::cout << "PASS: below threshold kept\n";
  }

  void test_identifier_hard_veto() {
    auto data = makeRows(
      {"id", "name"},
      {
        {"A001", "John Smith"},
        {"A002", "John Smith"}  // same name but different ID
      }
    );
    std::vector<ColumnType> types = {ColumnType::ID, ColumnType::NAME};
    auto result = weightedDeduplicate(data, types, 0.90);
    // Both rows should be kept — ID disagreement is a hard veto
    assert(result.data.size() == 3); // header + 2
    assert(result.rowsRemoved == 0);
    std::cout << "PASS: identifier hard veto prevents merge\n";
  }

  void test_empty_input() {
    std::vector<std::vector<std::string>> data;
    std::vector<ColumnType> types;
    auto result = weightedDeduplicate(data, types, 0.95);
    assert(result.data.empty());
    assert(result.rowsRemoved == 0);
    std::cout << "PASS: empty input returns empty\n";
  }

  void test_single_row() {
    auto data = makeRows(
      {"col1"},
      {{"only row"}}
    );
    std::vector<ColumnType> types = {ColumnType::GENERIC_TEXT};
    auto result = weightedDeduplicate(data, types, 0.95);
    assert(result.data.size() == 2); // header + 1
    assert(result.rowsRemoved == 0);
    std::cout << "PASS: single row returns same\n";
  }

  void run_all() {
    test_header_preserved();
    test_exact_duplicate_removed();
    test_different_rows_kept();
    test_near_duplicate_above_threshold_removed();
    test_below_threshold_kept();
    test_identifier_hard_veto();
    test_empty_input();
    test_single_row();
    std::cout << "\nAll weighted dedup tests passed (8/8)\n";
  }
};

int main() {
  WeightedDedupTest tests;
  tests.run_all();
  return 0;
}
