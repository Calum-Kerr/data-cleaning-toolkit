#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "column_type_detection.h"

class ColumnTypeDetectionTest {
public:
  std::vector<std::vector<std::string>> makeData(
      const std::vector<std::string>& headers,
      const std::vector<std::vector<std::string>>& rows) {
    std::vector<std::vector<std::string>> data;
    data.push_back(headers);
    for (const auto& row : rows) data.push_back(row);
    return data;
  }

  void test_email_detection() {
    auto data = makeData(
      {"email"},
      {{"alice@example.com"}, {"bob@domain.org"}, {"charlie@test.net"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::EMAIL);
    std::cout << "PASS: email detection\n";
  }

  void test_phone_detection() {
    auto data = makeData(
      {"phone"},
      {{"+1-555-1234"}, {"(555) 987-6543"}, {"+447700900123"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::PHONE);
    std::cout << "PASS: phone detection\n";
  }

  void test_url_detection() {
    auto data = makeData(
      {"website"},
      {{"http://example.com"}, {"https://test.org"}, {"www.site.com"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::URL);
    std::cout << "PASS: URL detection\n";
  }

  void test_date_detection() {
    auto data = makeData(
      {"date"},
      {{"2024-01-15"}, {"2023-12-31"}, {"2024-06-01"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::DATE);
    std::cout << "PASS: date detection (ISO format)\n";
  }

  void test_numeric_detection() {
    auto data = makeData(
      {"price"},
      {{"42"}, {"3.14"}, {"-7"}, {"1000"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::NUMERIC);
    std::cout << "PASS: numeric detection\n";
  }

  void test_boolean_detection() {
    auto data = makeData(
      {"active"},
      {{"true"}, {"false"}, {"yes"}, {"no"}, {"1"}, {"0"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::BOOLEAN);
    std::cout << "PASS: boolean detection\n";
  }

  void test_name_via_header_hint() {
    auto data = makeData(
      {"full_name"},
      {{"John Smith"}, {"Jane Doe"}, {"Bob Jones"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    assert(result.types[0] == ColumnType::NAME);
    std::cout << "PASS: name detection via header hint\n";
  }

  void test_generic_text_fallback() {
    auto data = makeData(
      {"field_x"},
      {{"abc"}, {"def"}, {"ghi"}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    // short, non-matching values without header hint → GENERIC_TEXT
    assert(result.types[0] == ColumnType::GENERIC_TEXT);
    std::cout << "PASS: GENERIC_TEXT fallback for unknown column\n";
  }

  void test_empty_column() {
    auto data = makeData(
      {"empty_col"},
      {{""}, {""}, {""}}
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 1);
    // empty sample → falls back to header hint → no match → GENERIC_TEXT
    assert(result.types[0] == ColumnType::GENERIC_TEXT);
    std::cout << "PASS: empty column → GENERIC_TEXT\n";
  }

  void test_multiple_columns() {
    auto data = makeData(
      {"email", "price", "name"},
      {
        {"alice@example.com", "42", "Alice Smith"},
        {"bob@domain.org", "99", "Bob Jones"}
      }
    );
    auto result = detectColumnTypes(data);
    assert(result.types.size() == 3);
    assert(result.types[0] == ColumnType::EMAIL);
    assert(result.types[1] == ColumnType::NUMERIC);
    assert(result.types[2] == ColumnType::NAME);
    assert(result.names.size() == 3);
    assert(result.names[0] == "email");
    assert(result.names[1] == "price");
    assert(result.names[2] == "name");
    std::cout << "PASS: multiple columns with mixed types\n";
  }

  void test_column_type_to_string() {
    assert(std::string(columnTypeToString(ColumnType::EMAIL)) == "EMAIL");
    assert(std::string(columnTypeToString(ColumnType::GENERIC_TEXT)) == "GENERIC_TEXT");
    std::cout << "PASS: columnTypeToString\n";
  }

  void test_type_weight() {
    assert(typeWeight(ColumnType::EMAIL) == 3.0);
    assert(typeWeight(ColumnType::FREE_TEXT) == 0.5);
    std::cout << "PASS: type weights\n";
  }

  void run_all() {
    test_email_detection();
    test_phone_detection();
    test_url_detection();
    test_date_detection();
    test_numeric_detection();
    test_boolean_detection();
    test_name_via_header_hint();
    test_generic_text_fallback();
    test_empty_column();
    test_multiple_columns();
    test_column_type_to_string();
    test_type_weight();
    std::cout << "\nAll column type detection tests passed (12/12)\n";
  }
};

int main() {
  ColumnTypeDetectionTest tests;
  tests.run_all();
  return 0;
}
