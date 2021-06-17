#include "json_lex.h"

bool JsonLexer::next(char c, char next_c, bool peek,
                     token_t &token, string &token_text) {
    // printf("char: %c in_string: %d escape = %d\n", c, in_string, escape);
    if ( in_string ) {
        if ( !escape and c == '\\' ) {
            escape = true;
            return false;
        } else if ( !escape and c == '"' ) {
            in_string = false;
            token = TOKEN_STRING;
        } else {
            val += c;
            escape = false;
            return false;
        }
    } else {
        if ( c == ' ' or c == '\t' or c == '\n' or c == '\r' ) {
            if ( !peek and c == '\n' ) {
                line_number++;
            }
            return false;
        } else if ( c == '{' ) {
            val = c;
            token = TOKEN_DICT_BEGIN;
        } else if ( c == '}' ) {
            val = c;
            token = TOKEN_DICT_END;
        } else if ( c == '[' ) {
            val = c;
            token = TOKEN_LIST_BEGIN;
        } else if ( c == ']' ) {
            val = c;
            token = TOKEN_LIST_END;
        } else if ( c == ':' ) {
            val = c;
            token = TOKEN_COLON;
        } else if ( c == ',' ) {
            val = c;
            token = TOKEN_COMMA;
        } else if ( c == '"' ) {
            in_string = true;
            val = "";
            return false;
        } else if ( !in_number and is_digit(c) ) {
            in_number = true;
            is_float = false;
            val = c;
            if ( is_real(next_c) ) {
                return false;
            }
            in_number = false;
            token = TOKEN_INT;
        } else if ( in_number and is_real(c) ) {
            val += c;
            if ( !is_digit(c) ) {
                is_float = true;
            }
            if ( is_real(next_c) ) {
                return false;
            }
            in_number = false;
            if ( ! is_float ) {
                token = TOKEN_INT;
            } else {
                token = TOKEN_REAL;
            }
        } else {
            printf("unhandled character: %d\n", c);
            return false;
        }
    }
    
    token_text = val;
    return true;
}

bool JsonLexer::get_token(token_t &next_token, string &next_text, bool peek ) {
    char c = 0;
    char next_c = 0;
    int n = index;
    while ( n < buf.length() ) {
        c = buf[n];
        if ( n < buf.length() ) {
            next_c = buf[n+1];
        } else {
            next_c = EOF;
        }
        n++;
        
        token_t token;
        string val;
        if ( next(c, next_c, peek, token, val) ) {
            // printf("token: %d val: %s\n", token, val.c_str());
            next_token = token;
            next_text = val;
            if ( !peek ) {
                index = n;
            }
            return true;
        }
    }
    return false;
}
