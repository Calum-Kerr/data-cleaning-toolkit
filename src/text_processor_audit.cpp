#include "audit_logger.h"
#include <string>
#include <vector>

class text_audit_logger {
private:
    audit_logger logger;
    int transformations_logged;

public:
    text_audit_logger() : transformations_logged(0) {
        logger.set_operation("process_text");
    }

    void log_transformation(const std::string& field,
                           const std::string& original,
                           const std::string& normalized,
                           const std::string& type,
                           float confidence) {
        audit::transformation_record trans;
        trans.type = type;
        trans.field = field;
        trans.original_value = original;
        trans.normalized_value = normalized;
        trans.confidence = confidence;
        trans.affected_rows = 1;

        logger.add_transformation(trans);
        transformations_logged++;
    }

    void set_hashes(const std::string& input,
                   const std::string& output) {
        logger.set_hashes(input, output);
    }

    void set_row_counts(int before, int after) {
        logger.set_row_counts(before, after);
    }

    const audit::audit_entry& finalize() {
        return logger.get_entry();
    }

    int get_transformation_count() const {
        return transformations_logged;
    }
};
