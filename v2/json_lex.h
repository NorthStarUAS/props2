// scan a byte stream and return tokens for simple json-style syntax

#pragma once

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

    inline void set_input(string &input_buf) {
        buf = input_buf;
        index = 0;
        line_number = 1;
    }
    inline bool peek_token(token_t &token, string &token_text) {
        return get_token(token, token_text, true);
    }
    bool get_token(token_t &token, string &token_text, bool peek = false); 
    inline int get_line_number() { return line_number; }

private:
    string buf = "";
    int len = 0;
    int index = 0;
    bool in_string = false;
    bool escape = false;
    bool in_number = false;
    bool is_float = false;
    string val = "";
    int line_number = 1;

    inline bool is_digit(char c) {
        return (c >= '0' and c <= '9') or (c == '-');
    }
    
    inline bool is_real(char c) {
        return (c >= '0' and c <= '9') or (c == '-') or (c == '.') or (c == 'e') or (c == 'E');
    }
    
    bool next(char c, char next_c, bool peek,
              token_t &token, string &token_text);

};
