#include <string>
#include <vector>
#include <ctime>
#include "time_safe.h"

namespace audit {
    struct transformation_record {
        std::string type;
        std::string field;
        std::string original_value;
        std::string normalized_value;
        float confidence;
        int affected_rows;
        std::string metadata;
    };

    struct audit_entry {
        std::string operation;
        std::string timestamp;
        std::string input_hash;
        std::string output_hash;
        int rows_before;
        int rows_after;
        std::vector<transformation_record> transformations;
        std::vector<std::string> errors;
    };
}

class audit_logger {
private:
    audit::audit_entry current_entry;

    std::string get_iso_timestamp() {
        time_t now = time(nullptr);
        char buffer[30];
        strftime(buffer, sizeof(buffer),
                 "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
        return std::string(buffer);
    }

public:
    audit_logger() {
        current_entry.timestamp = get_iso_timestamp();
    }

    void set_operation(const std::string& op) {
        current_entry.operation = op;
    }

    void set_hashes(const std::string& input,
                    const std::string& output) {
        current_entry.input_hash = input;
        current_entry.output_hash = output;
    }

    void add_transformation(const audit::transformation_record& trans) {
        current_entry.transformations.push_back(trans);
    }

    void set_row_counts(int before, int after) {
        current_entry.rows_before = before;
        current_entry.rows_after = after;
    }

    const audit::audit_entry& get_entry() const {
        return current_entry;
    }
};
