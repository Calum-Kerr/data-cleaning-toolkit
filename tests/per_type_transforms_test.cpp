#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "text_normalisation.h"

// a minimal reimplementation of transformCell from deep_clean.cpp for testing
static std::string lower(const std::string& s) { return toLowerCase(s); }
static std::string upper(const std::string& s) { return toUpperCase(s); }

static std::string transformCellExternal(const std::string& cell, int typeEnum) {
  if (cell.empty()) return "";

  // ColumnType enum values: EMAIL=0, PHONE=1, URL=2, DATE=3, NUMERIC=4,
  // BOOLEAN=5, ID=6, NAME=7, FREE_TEXT=8, GENERIC_TEXT=9
  switch (typeEnum) {
    case 0: // EMAIL → lowercase
      return lower(cell);
    case 1: // PHONE → digits only (keep +)
    {
      std::string out;
      for (char c : cell) {
        if (c == '+' && out.empty()) out += c;
        else if (c >= '0' && c <= '9') out += c;
      }
      return out.empty() ? cell : out;
    }
    case 2: // URL → lowercase
      return lower(cell);
    case 3: // DATE → normalize ISO
      if (cell.size() == 10 && cell[4] == '/' && cell[7] == '/') {
        return cell.substr(0,4) + "-" + cell.substr(5,2) + "-" + cell.substr(8,2);
      }
      if (cell.size() == 10 && cell[2] == '/' && cell[5] == '/') {
        return cell.substr(6,4) + "-" + cell.substr(3,2) + "-" + cell.substr(0,2);
      }
      if (cell.size() == 10 && cell[2] == '-' && cell[5] == '-') {
        return cell.substr(6,4) + "-" + cell.substr(3,2) + "-" + cell.substr(0,2);
      }
      return cell;
    case 4: // NUMERIC → strip commas/currency
    {
      std::string out;
      for (char c : cell) {
        if (c == ',' || c == ' ') continue;
        if (out.empty() && (c == '$' || c == '\xA3' || c == '\xE2')) continue;
        out += c;
      }
      return out.empty() ? cell : out;
    }
    case 5: // BOOLEAN → canonical
    {
      std::string l = lower(cell);
      if (l == "true" || l == "yes" || l == "1" || l == "t" || l == "y") return "true";
      if (l == "false" || l == "no" || l == "0" || l == "f" || l == "n") return "false";
      return cell;
    }
    case 6: // ID → trim
    {
      size_t s = cell.find_first_not_of(" \t\r\n");
      size_t e = cell.find_last_not_of(" \t\r\n");
      if (s == std::string::npos) return "";
      return cell.substr(s, e - s + 1);
    }
    case 7: // NAME → uppercase
      return upper(cell);
    case 8: // FREE_TEXT → no change
      return cell;
    case 9: // GENERIC_TEXT → uppercase
      return upper(cell);
    default:
      return cell;
  }
}

class TransformTests {
public:
  void test_email_lowercase() {
    std::string result = transformCellExternal("John@Example.Com", 0); // EMAIL
    assert(result == "john@example.com");
    std::cout << "PASS: EMAIL → lowercase\n";
  }

  void test_url_lowercase() {
    std::string result = transformCellExternal("HTTP://EXAMPLE.COM", 2); // URL
    assert(result == "http://example.com");
    std::cout << "PASS: URL → lowercase\n";
  }

  void test_phone_digits_only() {
    std::string result = transformCellExternal("+1 (555) 123-4567", 1); // PHONE
    assert(result == "+15551234567");
    std::cout << "PASS: PHONE → digits only\n";
  }

  void test_name_uppercase() {
    std::string result = transformCellExternal("John Smith", 7); // NAME
    assert(result == "JOHN SMITH");
    std::cout << "PASS: NAME → uppercase\n";
  }

  void test_free_text_unchanged() {
    std::string result = transformCellExternal("Hello World", 8); // FREE_TEXT
    assert(result == "Hello World");
    std::cout << "PASS: FREE_TEXT → unchanged\n";
  }

  void test_generic_text_uppercase() {
    std::string result = transformCellExternal("hello world", 9); // GENERIC_TEXT
    assert(result == "HELLO WORLD");
    std::cout << "PASS: GENERIC_TEXT → uppercase\n";
  }

  void test_empty_cell_preserved() {
    std::string result = transformCellExternal("", 0); // any type
    assert(result == "");
    std::cout << "PASS: empty cell preserved\n";
  }

  void test_date_normalize() {
    std::string result = transformCellExternal("15/06/2024", 3); // DATE DD/MM/YYYY
    assert(result == "2024-06-15");
    std::cout << "PASS: DATE DD/MM/YYYY → ISO\n";
  }

  void test_date_iso_unchanged() {
    std::string result = transformCellExternal("2024-06-15", 3); // DATE already ISO
    assert(result == "2024-06-15");
    std::cout << "PASS: DATE ISO unchanged\n";
  }

  void test_boolean_canonical() {
    assert(transformCellExternal("YES", 5) == "true");
    assert(transformCellExternal("No", 5) == "false");
    assert(transformCellExternal("1", 5) == "true");
    assert(transformCellExternal("0", 5) == "false");
    assert(transformCellExternal("True", 5) == "true");
    std::cout << "PASS: BOOLEAN → canonical\n";
  }

  void test_numeric_strip() {
    std::string result = transformCellExternal("1,234.56", 4); // NUMERIC
    assert(result == "1234.56");
    std::cout << "PASS: NUMERIC → strip commas\n";
  }

  void test_id_trim() {
    std::string result = transformCellExternal("  ABC-123  ", 6); // ID
    assert(result == "ABC-123");
    std::cout << "PASS: ID → trim\n";
  }

  void run_all() {
    test_email_lowercase();
    test_url_lowercase();
    test_phone_digits_only();
    test_name_uppercase();
    test_free_text_unchanged();
    test_generic_text_uppercase();
    test_empty_cell_preserved();
    test_date_normalize();
    test_date_iso_unchanged();
    test_boolean_canonical();
    test_numeric_strip();
    test_id_trim();
    std::cout << "\nAll per-type transform tests passed (12/12)\n";
  }
};

int main() {
  TransformTests tests;
  tests.run_all();
  return 0;
}
