#include "text_errors.h"
#include <string>
#include <cctype>

namespace whitespace {
    const char SPACE = ' ';
    const char TAB = '\t';
    const char LF = '\n';
    const char CR = '\r';
}

class whitespace_normalizer {
private:
    int transformations_count;

    void collapse_spaces(std::string& value) {
        std::string result;
        bool in_space = false;

        for (char c : value) {
            if (std::isspace(c)) {
                if (!in_space) {
                    result += whitespace::SPACE;
                    in_space = true;
                }
            } else {
                result += c;
                in_space = false;
            }
        }

        value = result;
    }

    void strip_edges(std::string& value) {
        size_t start = value.find_first_not_of(" \t\n\r");
        size_t end = value.find_last_not_of(" \t\n\r");

        if (start == std::string::npos) {
            value = "";
        } else {
            value = value.substr(start, end - start + 1);
        }
    }

    void normalize_line_endings(std::string& value) {
        std::string result;

        for (size_t i = 0; i < value.length(); i++) {
            if (value[i] == whitespace::CR) {
                if (i + 1 < value.length() &&
                    value[i + 1] == whitespace::LF) {
                    result += whitespace::LF;
                    i++;
                } else {
                    result += whitespace::LF;
                }
            } else {
                result += value[i];
            }
        }

        value = result;
    }

public:
    whitespace_normalizer() : transformations_count(0) {}

    std::string process(const std::string& original) {
        std::string result = original;

        strip_edges(result);
        collapse_spaces(result);
        normalize_line_endings(result);

        if (result != original) {
            transformations_count++;
        }

        return result;
    }

    int get_count() const {
        return transformations_count;
    }
};
