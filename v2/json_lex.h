// scan a byte stream and return tokens for simple json-style syntax

#include <string>
using std::string;

class JsonLexer {
    
public:
    enum token_t {
        TOKEN_NONE,
        TOKEN_DICT_BEGIN,
        TOKEN_DICT_END,
        TOKEN_LIST_BEGIN,
        TOKEN_LIST_END,
        TOKEN_COLON,
        TOKEN_COMMA,
        TOKEN_STRING,
        TOKEN_INT,
        TOKEN_REAL,
    };
    
    bool next(char c, char next_c, token_t *token, string *token_text);

private:
    bool in_string = false;
    bool escape = false;
    bool in_number = false;
    bool is_float = false;
    string val = "";

    inline bool is_digit(char c) {
        return (c >= '0' and c <= '9') or (c == '-');
    }
    inline bool is_real(char c) {
        return (c >= '0' and c <= '9') or (c == '-') or (c == '.') or (c == 'e') or (c == 'E');
    }
    
};
