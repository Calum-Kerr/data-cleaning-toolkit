#include <string>
#include <cctype>
#include <algorithm>

class case_standardizer {
private:
    bool is_all_caps(const std::string& value) {
        for (char c : value) {
            if (std::isalpha(c) && std::islower(c)) {
                return false;
            }
        }
        return true;
    }

    bool is_all_lower(const std::string& value) {
        for (char c : value) {
            if (std::isalpha(c) && std::isupper(c)) {
                return false;
            }
        }
        return true;
    }

    std::string to_title_case(const std::string& value) {
        std::string result;
        bool capitalize_next = true;

        for (char c : value) {
            if (std::isspace(c)) {
                result += c;
                capitalize_next = true;
            } else {
                if (capitalize_next && std::isalpha(c)) {
                    result += std::toupper(c);
                    capitalize_next = false;
                } else {
                    result += std::tolower(c);
                }
            }
        }

        return result;
    }

public:
    std::string standardize(const std::string& value) {
        if (value.empty()) {
            return value;
        }

        if (is_all_caps(value) && value.length() > 1) {
            return to_title_case(value);
        }

        if (is_all_lower(value) && std::isalpha(value[0])) {
            return to_title_case(value);
        }

        return value;
    }

    std::string remove_accents(const std::string& value) {
        // Placeholder for accent removal (preserves for now)
        return value;
    }
};
