#include "json_lex.h"

bool JsonLexer::next(char c, char next_c,
                       token_t *token, string *token_text) {
    // printf("char: %c in_string: %d escape = %d\n", c, in_string, escape);
    if ( in_string ) {
        if ( !escape and c == '\\' ) {
            escape = true;
            return false;
        } else if ( !escape and c == '"' ) {
            in_string = false;
            *token = TOKEN_STRING;
        } else {
            val += c;
            escape = false;
            return false;
        }
    } else {
        if ( c == ' ' or c == '\t' or c == '\n' or c == '\r' ) {
            return false;
        } else if ( c == '{' ) {
            val = c;
            *token = TOKEN_DICT_BEGIN;
        } else if ( c == '}' ) {
            val = c;
            *token = TOKEN_DICT_END;
        } else if ( c == '[' ) {
            val = c;
            *token = TOKEN_LIST_BEGIN;
        } else if ( c == ']' ) {
            val = c;
            *token = TOKEN_LIST_END;
        } else if ( c == ':' ) {
            val = c;
            *token = TOKEN_COLON;
        } else if ( c == ',' ) {
            val = c;
            *token = TOKEN_COMMA;
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
            *token = TOKEN_INT;
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
                *token = TOKEN_INT;
            } else {
                *token = TOKEN_REAL;
            }
        } else {
            printf("unhandled character: %d\n", c);
            return false;
        }
    }
    
    *token_text = val;
    return true;
}

int main() {
    JsonLexer slex;
    JsonLexer::token_t token;
    string val;

    char last_c = 0;
    while ( true ) {
        char c = getchar();
        if ( last_c == EOF ) {
            break;
        }
        if ( slex.next(last_c, c, &token, &val) ) {
            printf("token: %d val: %s\n", token, val.c_str());
        }
        last_c = c;
    }
}
