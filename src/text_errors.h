#ifndef TEXT_ERRORS_H
#define TEXT_ERRORS_H

namespace text_errors {
    // Success status
    const int OK = 0;

    // Text processor error codes
    const int INVALID_UTF8 = 1;
    const int AMBIGUOUS_DATE = 2;
    const int CONFLICTING_PATTERNS = 3;
    const int INCOMPLETE_PATTERN = 4;
    const int INVALID_EMAIL = 5;
    const int INVALID_PHONE = 6;
    const int INVALID_URL = 7;
    const int INVALID_ADDRESS = 8;
    const int MIXED_LANGUAGE = 9;
    const int ENCODING_MISMATCH = 10;

    // Fuzzy matching thresholds (0.0 to 1.0)
    const float SIMILARITY_THRESHOLD = 0.85f;
    const float LEVENSHTEIN_THRESHOLD = 0.80f;

    // String length limits
    const int MIN_WORD_LENGTH = 2;
    const int MAX_FIELD_LENGTH = 10000;

    // Pattern matching confidence thresholds
    const float CONFIDENCE_HIGH = 0.95f;
    const float CONFIDENCE_MEDIUM = 0.80f;
    const float CONFIDENCE_LOW = 0.60f;
}

#endif
