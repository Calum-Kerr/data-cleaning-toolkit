#include "parser_errors.h"
#include <string>

namespace null_standardization {
    const std::string NULL_PLACEHOLDER = "[NULL]";
    const std::string JSON_NULL = "null";
    const std::string JSON_UNDEFINED = "undefined";

    bool is_null_value(const std::string& value) {
        return value == JSON_NULL || value == JSON_UNDEFINED ||
               value.empty();
    }

    std::string standardize_null(const std::string& original_value) {
        if (is_null_value(original_value)) {
            return NULL_PLACEHOLDER;
        }
        return original_value;
    }

    struct null_transformation {
        std::string original_value;
        std::string standardized_value;
        int affected_rows;
        std::string original_hash;
    };
}

class null_standardizer {
private:
    int transformations_count;
    null_standardization::null_transformation transforms;

    void compute_hash(const std::string& value) {
        // Placeholder for SHA256 hash computation
        transforms.original_hash = "hash_" + value;
    }

public:
    null_standardizer() : transformations_count(0) {}

    std::string process(const std::string& value, int row_count) {
        std::string result = null_standardization::standardize_null(value);

        if (result != value) {
            transforms.original_value = value;
            transforms.standardized_value = result;
            transforms.affected_rows = row_count;
            compute_hash(value);
            transformations_count++;
        }

        return result;
    }

    int get_transformation_count() const {
        return transformations_count;
    }

    const null_standardization::null_transformation& get_last_transform() {
        return transforms;
    }
};
