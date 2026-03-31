#ifndef TEXT_PATTERNS_H
#define TEXT_PATTERNS_H

namespace text_patterns {
    // Phone patterns
    const char* PHONE_US =
        "(?:\\+?1[-.]?)?(?:\\(?\\d{3}\\)?[-.]?)?\\d{3}[-.]?\\d{4}";

    // Currency patterns
    const char* CURRENCY =
        "[\\$€£¥]\\s?[0-9]{1,3}(?:,?[0-9]{3})*(?:\\.[0-9]{2})?";

    // Email patterns (RFC 5322 simplified)
    const char* EMAIL =
        "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}";

    // URL patterns
    const char* URL =
        "(?:https?|ftp)://[^\\s]+";

    // ISO 8601 dates
    const char* DATE_ISO =
        "\\d{4}-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\\d|3[01])";

    // US dates (MM/DD/YYYY)
    const char* DATE_US =
        "(?:0[1-9]|1[0-2])[/-](?:0[1-9]|[12]\\d|3[01])[/-]\\d{4}";

    // EU dates (DD.MM.YYYY)
    const char* DATE_EU =
        "(?:0[1-9]|[12]\\d|3[01])[.-](?:0[1-9]|1[0-2])[.-]\\d{4}";

    // Street addresses
    const char* ADDRESS =
        "\\d+\\s+[A-Za-z]+\\s+(?:St|Street|Ave|Avenue|Rd|Road)";

    // Postal codes (US ZIP)
    const char* POSTAL_US =
        "\\d{5}(?:-\\d{4})?";

    // Postal codes (UK format)
    const char* POSTAL_UK =
        "[A-Z]{2}\\d{1,2}\\s?\\d[A-Z]{2}";
}

#endif
