#ifndef PARSER_ERRORS_H
#define PARSER_ERRORS_H

namespace parser_errors {
    // Success status
    const int OK = 0;

    // JSON/XML parsing errors
    const int UNCLOSED_STRING = 1;
    const int UNCLOSED_ARRAY = 2;
    const int UNCLOSED_OBJECT = 3;
    const int INVALID_ESCAPE = 4;
    const int INVALID_UTF8 = 5;
    const int DUPLICATE_KEY = 6;
    const int INVALID_NUMBER = 7;
    const int EXPECTED_COMMA = 8;
    const int EXPECTED_COLON = 9;
    const int INVALID_ROOT = 10;
    const int MALFORMED_TAG = 11;
    const int UNCLOSED_TAG = 12;
    const int INVALID_ENTITY = 13;
    const int UNCLOSED_CDATA = 14;

    // Transformation thresholds
    const float CONFIDENCE_HIGH = 0.95f;
    const float CONFIDENCE_MEDIUM = 0.80f;
    const float CONFIDENCE_LOW = 0.60f;

    // Size limits
    const int MAX_NESTING_DEPTH = 100;
    const int MAX_KEY_LENGTH = 1024;
    const int MAX_STRING_LENGTH = 1000000;
}

#endif
