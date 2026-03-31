#include "parser_state.h"
#include "parser_errors.h"
#include <string>
#include <vector>

struct xml_token {
    std::string name;
    std::string value;
    xml_state state;
    int position;
};

class xml_tokenizer {
private:
    const char* input;
    size_t position;
    xml_state current_state;
    std::vector<xml_token> tokens;

    void transition_state(char c) {
        switch (current_state) {
            case XML_START:
                current_state = (c == '<') ? IN_TAG : XML_START;
                break;
            case IN_TAG:
                current_state = (c == ' ') ? IN_ATTRIBUTE : IN_TAG;
                break;
            case IN_ATTRIBUTE:
                current_state = (c == '=') ? IN_TEXT : IN_ATTRIBUTE;
                break;
            case IN_TEXT:
                current_state = (c == '<') ? IN_TAG : IN_TEXT;
                break;
            case IN_CDATA:
                current_state = (c == ']') ? CLOSE_TAG : IN_CDATA;
                break;
            case IN_ENTITY:
                current_state = (c == ';') ? IN_TEXT : IN_ENTITY;
                break;
            case CLOSE_TAG:
                current_state = (c == '>') ? XML_START : CLOSE_TAG;
                break;
            default:
                break;
        }
    }

public:
    xml_tokenizer(const char* xml_input) :
        input(xml_input), position(0),
        current_state(XML_START) {}

    int tokenize() {
        while (input[position] != '\0') {
            char c = input[position];
            transition_state(c);
            position++;
        }
        return parser_errors::OK;
    }

    int get_error_code() const {
        if (current_state == IN_TAG) {
            return parser_errors::UNCLOSED_TAG;
        }
        return parser_errors::OK;
    }
};
