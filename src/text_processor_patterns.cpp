#include "text_patterns.h"
#include "text_errors.h"
#include <string>
#include <regex>
#include <vector>

struct pattern_match {
    std::string pattern_type;
    std::string matched_value;
    float confidence;
    std::string detected_format;
};

class pattern_detector {
private:
    std::vector<pattern_match> matches;

    bool try_match(const std::string& value,
                   const char* pattern,
                   const std::string& type,
                   float confidence,
                   const std::string& format) {
        try {
            std::regex regex_pattern(pattern);
            if (std::regex_search(value, regex_pattern)) {
                pattern_match match;
                match.pattern_type = type;
                match.matched_value = value;
                match.confidence = confidence;
                match.detected_format = format;
                matches.push_back(match);
                return true;
            }
        } catch (...) {
            return false;
        }
        return false;
    }

public:
    void detect_patterns(const std::string& value) {
        try_match(value, text_patterns::PHONE_US,
                  "phone", text_errors::CONFIDENCE_HIGH,
                  "us_phone");

        try_match(value, text_patterns::EMAIL,
                  "email", text_errors::CONFIDENCE_HIGH,
                  "email_address");

        try_match(value, text_patterns::CURRENCY,
                  "currency", text_errors::CONFIDENCE_MEDIUM,
                  "currency_amount");

        try_match(value, text_patterns::DATE_ISO,
                  "date", text_errors::CONFIDENCE_HIGH,
                  "iso_8601");

        try_match(value, text_patterns::URL,
                  "url", text_errors::CONFIDENCE_HIGH,
                  "web_url");
    }

    const std::vector<pattern_match>& get_matches() const {
        return matches;
    }

    bool has_pattern(const std::string& type) const {
        for (const auto& m : matches) {
            if (m.pattern_type == type) {
                return true;
            }
        }
        return false;
    }
};
