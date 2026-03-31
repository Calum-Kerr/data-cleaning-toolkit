#include <string>
#include <map>
#include <vector>

namespace json_extraction {
    const char SEPARATOR = '.';

    struct key_value_pair {
        std::string key;
        std::string value;
        int depth;
    };
}

class json_key_value_extractor {
private:
    std::map<std::string, std::string> extracted_values;
    std::vector<json_extraction::key_value_pair> all_pairs;
    int nesting_depth;

    void extract_at_depth(const std::string& key,
                          const std::string& value) {
        json_extraction::key_value_pair pair;
        pair.key = key;
        pair.value = value;
        pair.depth = nesting_depth;
        all_pairs.push_back(pair);
        extracted_values[key] = value;
    }

public:
    json_key_value_extractor() : nesting_depth(0) {}

    void extract_from_object(const std::string& obj_key,
                             const std::string& obj_value) {
        nesting_depth++;

        if (nesting_depth > 100) {
            return;  // Depth limit exceeded
        }

        extract_at_depth(obj_key, obj_value);
        nesting_depth--;
    }

    std::string get_value(const std::string& key) const {
        auto it = extracted_values.find(key);
        if (it != extracted_values.end()) {
            return it->second;
        }
        return "";
    }

    const std::vector<json_extraction::key_value_pair>&
    get_all_pairs() const {
        return all_pairs;
    }

    int get_extraction_count() const {
        return all_pairs.size();
    }
};
