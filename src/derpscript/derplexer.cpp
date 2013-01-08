// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   This software is provided 'as-is', without any express or implied
//   warranty. In no event will the authors be held liable for any
//   damages arising from the use of this software.
//
//   Permission is granted to anyone to use this software for any
//   purpose, including commercial applications, and to alter it and
//   redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must
//      not claim that you wrote the original software. If you use this
//      software in a product, an acknowledgment in the product
//      documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must
//      not be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//
// -------------------------- END HEADER -------------------------------------

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

#include "derperror.h"
#include "derperror_internal.h"
#include "derpconfig.h"
#include "derplexer.h"

namespace ExPop {

    // FIXME: This is from Lily Engine Utils. Get rid of this function
    // and use that version once everything is set up.
    static std::string stringUnescape(const std::string &str) {

        ostringstream outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            if(str[i] == '\\') {

                i++;

                if(i >= str.size()) break;

                switch(str[i]) {

                    case '\\':
                        outStr << "\\";
                        break;

                    case '"':
                        outStr << "\"";
                        break;

                    case 'n':
                        outStr << "\n";
                        break;

                    case 'r':
                        outStr << "\r";
                        break;

                    default:
                        // What the heck is this?
                        outStr << str[i];
                        break;

                }

            } else {

                outStr << str[i];

            }
        }

        return outStr.str();
    }


    static bool isWhitespace(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\t';
    }

    static bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }


    static DerpToken *parseNumber(
        const std::string &in,
        unsigned int &i,
        const std::string &fileName,
        unsigned int &lineNumber) {

        unsigned int start = i;
        bool isFloat = false;

        while(i < in.size() && (in[i] == '.' || isDigit(in[i]))) {
            if(in[i] == '.') {
                isFloat = true;
            }
            i++;
        }

        string numStr = in.substr(start, i-start);
        DerpToken *out = new DerpToken(
            numStr,
            isFloat ? DERPTOKEN_FLOAT : DERPTOKEN_INT,
            fileName,
            lineNumber);

        return out;

    }


    static DerpToken *parseString(
        const std::string &in,
        unsigned int &i,
        const std::string &fileName,
        unsigned int &lineNumber,
        DerpErrorState &errorState) {

        unsigned int start;
        bool escaped = false;

        // Skip first quote mark.
        i++;
        start = i;

        while(i < in.size()) {

            if(in[i] == '\"' && !escaped) break;

            if(in[i] == '\\') {
                escaped = !escaped;
            } else {
                escaped = false;
            }

            if(in[i] == '\n') {
                lineNumber++;
            }

            i++;

            if(i - start > DERP_MAX_STRING_LENGTH) {

                errorState.setFileAndLineDirect(
                    fileName,
                    lineNumber);

                errorState.addError(
                    "Exceeded max string length for literal string.");

                return NULL;
            }
        }

        string finalStr = stringUnescape(in.substr(start, i-start));
        DerpToken *out = new DerpToken(
            finalStr, DERPTOKEN_STRING,
            fileName, lineNumber);

        // Skip the last quote mark.
        i++;

        return out;

    }

    static bool isValidSymbolChar(char c, bool firstChar) {

        if((c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_'))
            return true;

        if(!firstChar &&
           (c >= '0' && c <= '9'))
            return true;

        return false;
    }

    static DerpToken *parseSymbolOrKeyword(
        const std::string &in,
        unsigned int &i,
        const std::string &fileName,
        unsigned int &lineNumber,
        DerpErrorState &errorState) {

        unsigned int start = i;

        while(i < in.size()) {

            if(!isValidSymbolChar(in[i], i == start)) {
                break;
            }

            i++;

            unsigned int len = i - start;

            if(len > DERP_MAX_TOKEN_LENGTH) {

                errorState.setFileAndLineDirect(
                    fileName,
                    lineNumber);

                errorState.addError(
                    "Exceeded maximum length for a single token");

                return NULL;
            }
        }

        string finalStr = in.substr(start, i-start);
        DerpToken *out = NULL;

        // FIXME: This is going to be slow and ugly.
        // TODO: Replace this with some sort of table or map lookup.
        if(finalStr == "function") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_FUNCTION, fileName, lineNumber);
        } else if(finalStr == "var") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_VAR, fileName, lineNumber);
        } else if(finalStr == "return") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_RETURN, fileName, lineNumber);
        } else if(finalStr == "break") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_BREAK, fileName, lineNumber);
        } else if(finalStr == "continue") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_CONTINUE, fileName, lineNumber);
        } else if(finalStr == "dbgout") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_DEBUGOUT, fileName, lineNumber);
        } else if(finalStr == "if") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_IF, fileName, lineNumber);
        } else if(finalStr == "else") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_ELSE, fileName, lineNumber);
        } else if(finalStr == "while") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_WHILE, fileName, lineNumber);
        } else if(finalStr == "for") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_FOR, fileName, lineNumber);
        } else if(finalStr == "do") {
            out = new DerpToken(
                finalStr, DERPTOKEN_KEYWORD_DO, fileName, lineNumber);
        } else {
            out = new DerpToken(
                finalStr, DERPTOKEN_SYMBOL, fileName, lineNumber);
        }

        return out;
    }

    static std::string strFromChar(char c) {
        char out[2];
        out[0] = c;
        out[1] = 0;
        return string(out);
    }

    bool derpGetTokens(
        const std::string &str,
        std::vector<DerpToken*> &outTokens,
        DerpErrorState &errorState,
        const std::string &fileName) {

        unsigned int i = 0;
        unsigned int lineNumber = 1;

        while(i < str.size()) {

            // Skip over whitespace.
            while(i < str.size() && isWhitespace(str[i])) {

                if(str[i] == '\n') lineNumber++;

                i++;
            }

            if(i >= str.size()) break;

            // Figure out token type with a big ugly if/else thinger.

            if(str[i] == '/' && (i + 1) < str.size() && str[i+1] == '/') {

                // C++ style comment. Just throw it away.
                i += 2;
                while(i < str.size()) {
                    if(str[i] == '\n') break;
                    i++;
                }

            } else if(str[i] == '\"') {

                // Found a literal quoted string.
                DerpToken *stringToken = parseString(
                    str, i, fileName, lineNumber, errorState);
                if(!stringToken) {
                    return false;
                }

                outTokens.push_back(stringToken);

            } else if(isDigit(str[i])) {

                // Found a number
                outTokens.push_back(parseNumber(str, i, fileName, lineNumber));

            } else if(isValidSymbolChar(str[i], true)) {

                // Found either a symbol or a keyword.
                DerpToken *symbolToken = parseSymbolOrKeyword(
                    str, i, fileName, lineNumber, errorState);
                if(!symbolToken) {
                    return false;
                }

                outTokens.push_back(symbolToken);

            } else {

                // Found some special character.

                // TODO: Replace this with some table-based lookup.

                bool foundTwoCharacterOperator = false;

                // First check all the two-character operators.

                if(str.size() > (i + 1)) {

                    char twoCharOp[3] = {
                        str[i], str[i+1], 0
                    };

                    if(string(":=") == twoCharOp ||
                       string("==") == twoCharOp ||
                       string("++") == twoCharOp ||
                       string("--") == twoCharOp ||
                       string(">=") == twoCharOp ||
                       string("<=") == twoCharOp) {

                        foundTwoCharacterOperator = true;
                        outTokens.push_back(new DerpToken(twoCharOp, DERPTOKEN_MATHOP, fileName, lineNumber));
                        i += 2;
                    }

                }

                // If it wasn't a two-character operator, try the
                // single-character operators.

                if(!foundTwoCharacterOperator) {

                    switch(str[i]) {

                        case '{':

                            // Found an open curly.
                            outTokens.push_back(new DerpToken("{", DERPTOKEN_OPENCURLY, fileName, lineNumber));
                            i++;
                            break;

                        case '}':

                            // Found a closing curly.
                            outTokens.push_back(new DerpToken("}", DERPTOKEN_CLOSECURLY, fileName, lineNumber));
                            i++;
                            break;

                        case '[':

                            // Found an open curly.
                            outTokens.push_back(new DerpToken("[", DERPTOKEN_OPENBRACKET, fileName, lineNumber));
                            i++;
                            break;

                        case ']':

                            // Found a closing curly.
                            outTokens.push_back(new DerpToken("]", DERPTOKEN_CLOSEBRACKET, fileName, lineNumber));
                            i++;
                            break;

                        case '(':

                            // Found an open parenthesis.
                            outTokens.push_back(new DerpToken("(", DERPTOKEN_OPENPAREN, fileName, lineNumber));
                            i++;
                            break;

                        case ')':

                            // Found a closing parenthesis.
                            outTokens.push_back(new DerpToken("(", DERPTOKEN_CLOSEPAREN, fileName, lineNumber));
                            i++;
                            break;

                        case ';':

                            // End statement.
                            outTokens.push_back(new DerpToken(";", DERPTOKEN_ENDSTATEMENT, fileName, lineNumber));
                            i++;
                            break;

                        case ',':
                            outTokens.push_back(new DerpToken(",", DERPTOKEN_COMMA, fileName, lineNumber));
                            i++;
                            break;

                        case '+':
                        case '-':
                        case '*':
                        case '/':
                        case '!':
                        case '=':
                        case '<':
                        case '>':

                            // Found math operator.
                            outTokens.push_back(new DerpToken(strFromChar(str[i]), DERPTOKEN_MATHOP, fileName, lineNumber));
                            i++;
                            break;

                        default:

                            // I have no idea what to do with this. Toss an error and return.

                            errorState.setFileAndLineDirect(
                                fileName,
                                lineNumber);

                            errorState.addError(
                                derpSprintf("Bad token: \"%s\"", str[i]));

                            return false;
                    }

                }

            }

            if(outTokens.size() > DERP_MAX_TOKENS) {

                errorState.setFileAndLineDirect(
                    fileName,
                    lineNumber);

                errorState.addError(
                    "Exceeded maximum number of tokens.");

                return false;
            }
        }

        return true;
    }

    const struct {
        DerpTokenType type;
        const char *str;
    } derpTokenToStringMappings[] = {

        { DERPTOKEN_OPENPAREN,        "'('" },
        { DERPTOKEN_CLOSEPAREN,       "')'" },
        { DERPTOKEN_OPENCURLY,        "'{'" },
        { DERPTOKEN_CLOSECURLY,       "'}'" },
        { DERPTOKEN_OPENBRACKET,      "'['" },
        { DERPTOKEN_CLOSEBRACKET,     "']'" },
        { DERPTOKEN_ENDSTATEMENT,     "';'" },
        { DERPTOKEN_MATHOP,           "math operator" },
        { DERPTOKEN_COMMA,            "','" },
        { DERPTOKEN_INT,              "integer" },
        { DERPTOKEN_FLOAT,            "float" },
        { DERPTOKEN_STRING,           "string" },
        { DERPTOKEN_KEYWORD_FUNCTION, "'function'" },
        { DERPTOKEN_KEYWORD_VAR,      "'var'" },
        { DERPTOKEN_KEYWORD_BREAK,    "'break'" },
        { DERPTOKEN_KEYWORD_CONTINUE, "'continue'" },
        { DERPTOKEN_KEYWORD_RETURN,   "'return'" },
        { DERPTOKEN_KEYWORD_DEBUGOUT, "'debugout'" },
        { DERPTOKEN_KEYWORD_IF,       "'if'" },
        { DERPTOKEN_KEYWORD_ELSE,     "'else'" },
        { DERPTOKEN_KEYWORD_WHILE,    "'while'" },
        { DERPTOKEN_SYMBOL,           "symbol" },
        { DERPTOKEN_JUNK,             NULL },

    };

    std::string derpTokenTypeToString(DerpTokenType t) {
        unsigned int i = 0;
        while(derpTokenToStringMappings[i].str) {

            if(t == derpTokenToStringMappings[i].type) {
                return derpTokenToStringMappings[i].str;
            }

            i++;
        }
        return "unknown";
    }
}

