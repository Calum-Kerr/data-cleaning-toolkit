#include <string>
#include <vector>
#include <map>

namespace json_flattening {
    const std::string SEPARATOR = "_";
    const int MAX_DEPTH = 10;

    struct flattened_field {
        std::string flattened_key;
        std::string value;
        int original_depth;
        std::string flattening_strategy;
    };
}

class json_flattener {
private:
    std::vector<json_flattening::flattened_field> flattened;
    int current_depth;

    void flatten_nested(const std::string& prefix,
                        const std::string& key,
                        const std::string& value) {
        if (current_depth > json_flattening::MAX_DEPTH) {
            return;
        }

        std::string flattened_key = prefix.empty() ? key :
                                    prefix + json_flattening::SEPARATOR + key;

        json_flattening::flattened_field field;
        field.flattened_key = flattened_key;
        field.value = value;
        field.original_depth = current_depth;
        field.flattening_strategy = "nested_object";
        flattened.push_back(field);
    }

public:
    json_flattener() : current_depth(0) {}

    void flatten(const std::string& prefix,
                 const std::string& key,
                 const std::string& value) {
        current_depth++;
        flatten_nested(prefix, key, value);
        current_depth--;
    }

    const std::vector<json_flattening::flattened_field>&
    get_flattened_fields() const {
        return flattened;
    }

    int get_field_count() const {
        return flattened.size();
    }
};
