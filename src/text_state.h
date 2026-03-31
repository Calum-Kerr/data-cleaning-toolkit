#ifndef TEXT_STATE_H
#define TEXT_STATE_H

// Text processor state machine
enum text_state {
    TEXT_START,         // Beginning of field value
    IN_WHITESPACE,      // Consuming whitespace (spaces, tabs, newlines)
    IN_WORD,            // Consuming word characters (letters, numbers, underscores)
    IN_NUMBER,          // Consuming numeric value (including . - for ranges)
    IN_SPECIAL,         // Consuming special characters (@, #, +, etc.)
    ESCAPE_SEQUENCE,    // After backslash, handle escaped character
    PATTERN_DETECTED    // Pattern matched (phone, email, currency, etc.)
};

#endif
