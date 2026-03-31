#include <string>
#include <cctype>
#include <regex>

class language_detector {
public:
    std::string detect_primary_language(const std::string& value) {
        // Simple detection: count Latin vs other scripts
        int latin_count = 0;
        for (char c : value) {
            if (std::isalpha(c) && static_cast<unsigned char>(c) < 128) {
                latin_count++;
            }
        }

        int alpha_count = 0;
        for (char c : value) {
            if (std::isalpha(c)) {
                alpha_count++;
            }
        }

        if (alpha_count == 0) {
            return "none";
        }

        float latin_ratio = (float)latin_count / alpha_count;
        return (latin_ratio > 0.8f) ? "english" : "mixed";
    }

    bool is_code_switched(const std::string& value) {
        std::string lang = detect_primary_language(value);
        return lang == "mixed";
    }
};

class numerical_standardizer {
private:
    bool is_negative(const std::string& value) {
        return value.find('-') != std::string::npos ||
               value.find('(') != std::string::npos;
    }

    std::string normalize_separators(const std::string& value) {
        // Convert comma to dot for European format
        std::string result = value;
        size_t dot_pos = result.find('.');
        size_t comma_pos = result.find(',');

        if (comma_pos < dot_pos) {
            // European format: replace comma with dot
            result.replace(comma_pos, 1, ".");
        }

        return result;
    }

public:
    std::string standardize(const std::string& value) {
        std::string result = normalize_separators(value);

        // Remove separators (thousands)
        result.erase(
            std::remove(result.begin(), result.end(), ','),
            result.end());

        return result;
    }
};
