#include <string>
using std::string;

#include "json_parse.h"

void JsonParser::info(const string msg) {
    printf("info: %d %s\n", lex.get_line_number(), msg.c_str());
}

void JsonParser::error(const string msg) {
    printf("Parse error line: %d %s\n", lex.get_line_number(), msg.c_str());
}

bool JsonParser::match_token(JsonLexer::token_t token_ref) {
    JsonLexer::token_t token;
    string val;
    if ( lex.get_token(token, val) ) {
        if ( token_ref == token ) {
            return true;
        }
        printf("Unexpected token: %s\n", val.c_str());
        error("");
    }
    return false;
}

bool JsonParser::match_statement() {
    info("match statement");
    JsonLexer::token_t token;
    string val;
    match_token(JsonLexer::TOKEN_STRING);
    match_token(JsonLexer::TOKEN_COLON);
    match_value();
    return true;
}

bool JsonParser::match_value() {
    info("match value");
    JsonLexer::token_t token;
    string val;
    if ( lex.peek_token(token, val) ) {
        if ( token == JsonLexer::TOKEN_DICT_BEGIN ) {
            match_dict();
        } else if ( token == JsonLexer::TOKEN_LIST_BEGIN ) {
            match_list();
        } else if ( token == JsonLexer::TOKEN_STRING or token == JsonLexer::TOKEN_INT or token == JsonLexer::TOKEN_REAL ) {
            match_literal();
        } else {
            error("Key: Value statement improperly formed");
            return false;
        }
        return true;
    }
    return false;
}

bool JsonParser::match_dict() {
    info("match_dict");
    match_token(JsonLexer::TOKEN_DICT_BEGIN);
    JsonLexer::token_t token;
    string val;
    lex.peek_token(token, val);
    if ( token != JsonLexer::TOKEN_DICT_END ) {
        bool done = false;
        while ( !done ) {
            if ( match_statement() ) {
                if ( lex.peek_token(token, val) ) {
                    if ( token == JsonLexer::TOKEN_COMMA ) {
                        lex.get_token(token, val);
                    } else {
                        done = true;
                    }
                }
            }
        }
    }
    match_token(JsonLexer::TOKEN_DICT_END);
    return true;
}

bool JsonParser::match_list() {
    info("match list");
    match_token(JsonLexer::TOKEN_LIST_BEGIN);
    bool done = false;
    while ( !done ) {
        if ( match_value() ) {
            JsonLexer::token_t token;
            string val;
            if ( lex.peek_token(token, val) ) {
                if ( token == JsonLexer::TOKEN_COMMA ) {
                    lex.get_token(token, val);
                    printf("found comma ...\n");
                } else {
                    printf("done ...\n");
                    done = true;
                }
            } else {
                done = true;
            }
        }
    }
    match_token(JsonLexer::TOKEN_LIST_END);
    printf("match_list end\n");
    return true;
}

bool JsonParser::match_literal() {
    info("match literal");
    JsonLexer::token_t token;
    string val;
    if ( !lex.peek_token(token, val) ) {
        return false;
    }
    lex.get_token(token, val);
    printf("  %s\n", val.c_str());
    return true;
}

bool JsonParser::parse(string &json_text) {
    info("begin parse\n");
    JsonLexer::token_t token;
    string val;

    lex.set_input(json_text);

    if ( false ) {
        // test: dump the list of tokens
        while ( lex.get_token(token, val) ) {
            printf("token: %d val: %s\n", token, val.c_str());
        }
    }

    match_token(JsonLexer::TOKEN_DICT_BEGIN);
    bool done = false;
    while ( !done ) {
        if ( match_statement() ) {
            JsonLexer::token_t token;
            string val;
            if ( lex.peek_token(token, val) ) {
                if ( token == JsonLexer::TOKEN_COMMA ) {
                    lex.get_token(token, val);
                    printf("found comma ...\n");
                } else {
                    printf("done ...\n");
                    done = true;
                }
            } else {
                done = true;
            }
        }
    }
    match_token(JsonLexer::TOKEN_DICT_END);

    return true;
}

int main() {
    JsonParser jp;

    // suck in all the input
    string input_buf = "";
    while ( true ) {
        char c = getchar();
        if ( c == EOF ) {
            break;
        }
        input_buf += c;
    }

    jp.parse(input_buf);
}
