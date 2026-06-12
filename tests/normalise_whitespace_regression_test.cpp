#include <cassert>
#include <iostream>
#include <string>

// declarations from text_normalisation.h (included via backend search path)
std::string normaliseWhitespace(const std::string& text);
std::string standardiseNullValues(const std::string& text);
std::string toUpperCase(const std::string& text);
std::string toLowerCase(const std::string& text);

class NormaliseWhitespaceRegressionTest {
public:
  void test_collapse_internal_spaces() {
    std::string result = normaliseWhitespace("Hello   World");
    assert(result == "Hello World");
    std::cout << "PASS: collapse internal spaces\n";
  }

  void test_trim_edges() {
    std::string result = normaliseWhitespace("  Hello World  ");
    assert(result == "Hello World");
    std::cout << "PASS: trim edges\n";
  }

  void test_handle_crlf() {
    std::string result = normaliseWhitespace("Hello\r\nWorld");
    assert(result == "Hello World");
    std::cout << "PASS: handle CRLF\n";
  }

  void test_handle_tab() {
    std::string result = normaliseWhitespace("Hello\tWorld");
    assert(result == "Hello World");
    std::cout << "PASS: handle tab\n";
  }

  void test_no_case_change() {
    // REGRESSION: normaliseWhitespace must NOT change case
    std::string result = normaliseWhitespace("John@Example.Com");
    assert(result == "John@Example.Com");
    std::cout << "PASS: no case change (regression)\n";
  }

  void test_mixed_whitespace_runs() {
    std::string result = normaliseWhitespace("Hello \t \r\n World");
    assert(result == "Hello World");
    std::cout << "PASS: mixed whitespace runs\n";
  }

  void test_empty_becomes_empty() {
    std::string result = normaliseWhitespace("");
    assert(result == "");
    std::cout << "PASS: empty stays empty\n";
  }

  void test_all_whitespace_becomes_empty() {
    std::string result = normaliseWhitespace("   \t  \r\n  ");
    assert(result == "");
    std::cout << "PASS: all whitespace becomes empty\n";
  }

  // --- standardiseNullValues tests ---

  void test_na_upper() {
    std::string result = standardiseNullValues("N/A");
    assert(result == "");
    std::cout << "PASS: N/A -> empty\n";
  }

  void test_na_lower() {
    std::string result = standardiseNullValues("n/a");
    assert(result == "");
    std::cout << "PASS: n/a (lowercase) -> empty\n";
  }

  void test_na_no_slash() {
    std::string result = standardiseNullValues("NA");
    assert(result == "");
    std::cout << "PASS: NA -> empty\n";
  }

  void test_null_value() {
    std::string result = standardiseNullValues("NULL");
    assert(result == "");
    std::cout << "PASS: NULL -> empty\n";
  }

  void test_null_lowercase() {
    std::string result = standardiseNullValues("null");
    assert(result == "");
    std::cout << "PASS: null (lowercase) -> empty\n";
  }

  void test_none_value() {
    std::string result = standardiseNullValues("NONE");
    assert(result == "");
    std::cout << "PASS: NONE -> empty\n";
  }

  void test_nil_value() {
    std::string result = standardiseNullValues("NIL");
    assert(result == "");
    std::cout << "PASS: NIL -> empty\n";
  }

  void test_missing_value() {
    std::string result = standardiseNullValues("MISSING");
    assert(result == "");
    std::cout << "PASS: MISSING -> empty\n";
  }

  void test_nan_value() {
    std::string result = standardiseNullValues("NaN");
    assert(result == "");
    std::cout << "PASS: NaN -> empty\n";
  }

  void test_dash_value() {
    std::string result = standardiseNullValues("--");
    assert(result == "");
    std::cout << "PASS: -- -> empty\n";
  }

  void test_hyphen_value() {
    std::string result = standardiseNullValues("-");
    assert(result == "");
    std::cout << "PASS: - -> empty\n";
  }

  void test_question_value() {
    std::string result = standardiseNullValues("?");
    assert(result == "");
    std::cout << "PASS: ? -> empty\n";
  }

  void test_tilde_value() {
    std::string result = standardiseNullValues("~");
    assert(result == "");
    std::cout << "PASS: ~ -> empty\n";
  }

  void test_non_null_preserved() {
    std::string result = standardiseNullValues("hello");
    assert(result == "hello");
    std::cout << "PASS: non-null value preserved\n";
  }

  void test_non_null_case_preserved() {
    // The returned value should keep its original case
    std::string result = standardiseNullValues("Hello World");
    assert(result == "Hello World");
    std::cout << "PASS: non-null case preserved\n";
  }

  void test_null_with_whitespace() {
    std::string result = standardiseNullValues("  N/A  ");
    assert(result == "");
    std::cout << "PASS: N/A with surrounding whitespace -> empty\n";
  }

  void run_all() {
    test_collapse_internal_spaces();
    test_trim_edges();
    test_handle_crlf();
    test_handle_tab();
    test_no_case_change();
    test_mixed_whitespace_runs();
    test_empty_becomes_empty();
    test_all_whitespace_becomes_empty();
    test_na_upper();
    test_na_lower();
    test_na_no_slash();
    test_null_value();
    test_null_lowercase();
    test_none_value();
    test_nil_value();
    test_missing_value();
    test_nan_value();
    test_dash_value();
    test_hyphen_value();
    test_question_value();
    test_tilde_value();
    test_non_null_preserved();
    test_non_null_case_preserved();
    test_null_with_whitespace();
    std::cout << "\nAll normaliseWhitespace regression tests passed (24/24)\n";
  }
};

int main() {
  NormaliseWhitespaceRegressionTest tests;
  tests.run_all();
  return 0;
}
