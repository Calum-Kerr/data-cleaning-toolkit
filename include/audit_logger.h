#pragma once

#include <string>
#include <vector>

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
    std::string get_iso_timestamp();

public:
    audit_logger();
    void set_operation(const std::string& op);
    void set_hashes(const std::string& input, const std::string& output);
    void add_transformation(const audit::transformation_record& trans);
    void set_row_counts(int before, int after);
    const audit::audit_entry& get_entry() const;
};
