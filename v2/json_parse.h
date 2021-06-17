// parse a simple json-style format

#pragma once

#include "json_lex.h"

class JsonParser {
    
public:
    bool parse(string &json_text);
    
private:
    JsonLexer lex;
    
    bool match_statement();
    bool match_value();
    bool match_dict();
    bool match_list();
    bool match_literal();
    bool match_token(JsonLexer::token_t token);
    void info(const string msg);
    void error(const string msg);
};
