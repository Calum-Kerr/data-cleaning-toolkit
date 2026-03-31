#include "parser_state.h"
#include "parser_errors.h"
#include <string>
#include <vector>

struct json_token {
    std::string value;
    json_state state;
    int position;
};

class json_tokenizer {
private:
    const char* input;
    size_t position;
    json_state current_state;
    std::vector<json_token> tokens;

    void transition_state(char c) {
        switch (current_state) {
            case JSON_START:
                current_state = (c == '{') ? IN_OBJECT : IN_ARRAY;
                break;
            case IN_OBJECT:
                current_state = (c == '"') ? IN_STRING : IN_OBJECT;
                break;
            case IN_ARRAY:
                current_state = (c == '"') ? IN_STRING : IN_NUMBER;
                break;
            case IN_STRING:
                current_state = (c == '\\') ? ESCAPE_SEQUENCE : IN_STRING;
                break;
            case ESCAPE_SEQUENCE:
                current_state = IN_STRING;
                break;
            case IN_NUMBER:
                current_state = (c == ',') ? VALUE_EXPECTED : IN_NUMBER;
                break;
            default:
                break;
        }
    }

public:
    json_tokenizer(const char* json_input) :
        input(json_input), position(0),
        current_state(JSON_START) {}

    int tokenize() {
        while (input[position] != '\0') {
            char c = input[position];
            transition_state(c);
            position++;
        }
        return parser_errors::OK;
    }

    int get_error_code() const {
        if (current_state == IN_STRING) {
            return parser_errors::UNCLOSED_STRING;
        }
        if (current_state == IN_OBJECT) {
            return parser_errors::UNCLOSED_OBJECT;
        }
        return parser_errors::OK;
    }
};
