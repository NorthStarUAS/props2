#include <string>
using std::string;

#include "json_parser.h"

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
