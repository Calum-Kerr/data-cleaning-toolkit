#ifndef PARSER_STATE_H
#define PARSER_STATE_H

// JSON parser state machine
enum json_state {
    JSON_START,         // Beginning of input, expect { or [
    IN_OBJECT,          // Inside {}, expect key or }
    IN_ARRAY,           // Inside [], expect value or ]
    IN_STRING,          // Inside quoted value, consume until "
    ESCAPE_SEQUENCE,    // After \, handle escaped character
    IN_NUMBER,          // Consuming numeric value
    KEY_EXPECTED,       // After comma in object, expect key
    VALUE_EXPECTED      // After :, expect value
};

// XML parser state machine
enum xml_state {
    XML_START,          // Beginning of input
    IN_TAG,             // Inside < >, parsing tag name
    IN_ATTRIBUTE,       // Parsing attribute key=value
    IN_TEXT,            // Consuming element text content
    IN_CDATA,           // Inside <![CDATA[ ]] section
    IN_ENTITY,          // Parsing &entity; reference
    CLOSE_TAG           // Parsing </tag>
};

#endif
