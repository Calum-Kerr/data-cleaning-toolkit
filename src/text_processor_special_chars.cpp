#include <string>
#include <map>

namespace special_chars {
    const char SMART_QUOTE_LEFT = '\u201c';
    const char SMART_QUOTE_RIGHT = '\u201d';
    const char STANDARD_QUOTE = '"';
    const char ELLIPSIS = '\u2026';
    const char ELLIPSIS_DOTS = '.';
}

class special_character_normalizer {
private:
    std::map<std::string, std::string> replacements;

    void build_replacement_map() {
        // Quotes
        replacements["\u201c"] = "\"";
        replacements["\u201d"] = "\"";
        replacements["\u2018"] = "'";
        replacements["\u2019"] = "'";

        // Dashes
        replacements["\u2013"] = "-";  // en-dash
        replacements["\u2014"] = "-";  // em-dash

        // Ellipsis
        replacements["\u2026"] = "...";

        // HTML entities
        replacements["&amp;"] = "&";
        replacements["&lt;"] = "<";
        replacements["&gt;"] = ">";
        replacements["&nbsp;"] = " ";
        replacements["&quot;"] = "\"";
    }

public:
    special_character_normalizer() {
        build_replacement_map();
    }

    std::string normalize(const std::string& value) {
        std::string result = value;

        for (const auto& [old_char, new_char] : replacements) {
            size_t pos = 0;
            while ((pos = result.find(old_char, pos)) !=
                   std::string::npos) {
                result.replace(pos, old_char.length(), new_char);
                pos += new_char.length();
            }
        }

        return result;
    }
};
