#include "parserjsonmacros.h"

#include "malstring.h"
#include "filesystem.h"
#include "lilyparser.h"

#include <cassert>
#include <string>
using namespace std;

namespace ExPop {

    // ----------------------------------------------------------------------
    //  Tokenization
    // ----------------------------------------------------------------------

    enum JsonParserTokenType {
        JSPT_UNKNOWN,

        JSPT_OPENCURLY,
        JSPT_CLOSECURLY,

        JSPT_LEFTBRACKET,
        JSPT_RIGHTBRACKET,

        JSPT_COLON,
        JSPT_COMMA,

        JSPT_STRING,

        JSPT_NULL,
    };

    static std::string charToString(char c) {
        char s1[2] = " ";
        s1[0] = c;
        return string(s1);
    }

    static bool isNumber(char c) {
        if(c >= '0' && c <= '9') return true;
        if(c == '-' || c == '.') return true;
        return false;
    }

    static void tokenizeJson(
        const std::string &str,
        std::string &errorStr,
        vector<ParserToken*> &tokens) {

        unsigned int ptr = 0;
        unsigned int lineNumber = 1;

        while(ptr < str.size()) {

            char c = str[ptr];

            if(c == '{' || c == '}') {

                tokens.push_back(
                    new ParserToken(
                        charToString(c),
                        c == '{' ? JSPT_OPENCURLY : JSPT_CLOSECURLY,
                        lineNumber));

            } else if(c == '[' || c == ']') {

                tokens.push_back(
                    new ParserToken(
                        charToString(c),
                        c == '[' ? JSPT_LEFTBRACKET : JSPT_RIGHTBRACKET,
                        lineNumber));

            } else if(c == ':') {

                tokens.push_back(
                    new ParserToken(
                        charToString(c),
                        JSPT_COLON,
                        lineNumber));

            } else if(c == ',') {

                tokens.push_back(
                    new ParserToken(
                        charToString(c),
                        JSPT_COMMA,
                        lineNumber));

            } else if(isWhiteSpace(c)) {

                if(c == '\n') {
                    lineNumber++;
                }

            } else if(c == '"') {

                // Quoted strings.

                unsigned int startPt = ptr;

                // Skip the opening quote.
                ptr++;

                unsigned int numConsecutiveSlashes = 0;
                while(ptr < str.size()) {

                    if(str[ptr] == '\\') {
                        numConsecutiveSlashes++;
                    } else {
                        numConsecutiveSlashes = 0;
                    }

                    if(str[ptr] == '"') {
                        if(numConsecutiveSlashes % 2 == 0) {
                            break;
                        }
                    }

                    ptr++;
                }

                string quotedStr = str.substr(
                    startPt + 1,
                    ptr - startPt - 1);

                quotedStr = stringUnescape(quotedStr);

                tokens.push_back(
                    new ParserToken(
                        quotedStr,
                        JSPT_STRING,
                        lineNumber));

            } else if(
                str.substr(ptr, 4) == "true" ||
                str.substr(ptr, 5) == "false") {

                tokens.push_back(
                    new ParserToken(
                        str[ptr] == 't' ? "true" : "false",
                        JSPT_STRING,
                        lineNumber));

                ptr +=
                    tokens[tokens.size() - 1]->str.size() - 1;

            } else if(str.substr(ptr, 4) == "null") {

                tokens.push_back(
                    new ParserToken(
                        "null",
                        JSPT_NULL,
                        lineNumber));
                ptr += 3; // strlen("null") - 1

            } else if(isNumber(c)) {

                // FIXME: This is too generous with anything
                // containing 'e'. It will not spit out an error when
                // it should.
                unsigned int startPt = ptr;

                // Keep going as long as we're seeing number-like
                // things.
                while(ptr < str.size() && (isNumber(str[ptr]) || str[ptr] == 'e')) {
                    ptr++;
                }

                tokens.push_back(
                    new ParserToken(
                        str.substr(startPt, ptr - startPt),
                        JSPT_STRING,
                        lineNumber));

                // Went too far. Go back one. (To counteract the ptr++
                // at the end.)
                ptr--;

            } else {

                ostringstream errorStream;
                errorStream << lineNumber << ": " <<
                    "Unrecognized token starting with '" << c << "'"<< endl;
                errorStr = errorStr + errorStream.str();
                return;
            }

            ptr++;
        }

    }

    // ----------------------------------------------------------------------
    //  Parsing
    // ----------------------------------------------------------------------

    static int getTokenTypeSafe(
    const vector<ParserToken*> &tokens,
        unsigned int i) {

        if(i >= tokens.size()) {
            return JSPT_UNKNOWN;
        }

        return tokens[i]->type;
    }

    static unsigned int getTokenLineSafe(
        const vector<ParserToken*> &tokens,
        unsigned int i) {

        if(!tokens.size())
            return 0;

        if(i >= tokens.size()) {
            return tokens[tokens.size() - 1]->lineNumber;
        }

        return tokens[i]->lineNumber;
    }

    static ParserNode *parseJsonObject(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr);

    static ParserNode *parseJsonArray(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr);

    static ParserNode *parseJsonThingy(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr) {

        if(TOKEN_TYPE(0) == JSPT_OPENCURLY) {
            return parseJsonObject(tokens, i, errorStr);

        } else if(TOKEN_TYPE(0) == JSPT_LEFTBRACKET) {
            return parseJsonArray(tokens, i, errorStr);

        } else if(TOKEN_TYPE(0) == JSPT_NULL) {
            ParserNode *newNode = new ParserNode();
            newNode->setName("_null");
            i++;
            return newNode;

        } else {
            ERROR("Unknown token type");
            i++;
            return new ParserNode();
        }
    }

    static ParserNode *parseJsonObject(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr) {

        // Expect and skip opening curly brace.
        assert(TOKEN_TYPE(0) == JSPT_OPENCURLY);
        i++;

        ParserNode *node = new ParserNode();

        while(
            i < tokens.size() &&
            TOKEN_TYPE(0) != JSPT_CLOSECURLY) {

            if(TOKEN_TYPE(0) == JSPT_COMMA) {

                // Skip commas.
                i++;

            } else if(TOKEN_TYPE(0) == JSPT_STRING &&
                      TOKEN_TYPE(1) == JSPT_COLON) {

                // Objects and attributes.

                string name = tokens[i]->str;
                i += 2;

                if(TOKEN_TYPE(0) == JSPT_STRING) {

                    node->setStringValue(
                        name,
                        tokens[i]->str);
                    i++;

                } else {

                    ParserNode *newNode =
                        parseJsonThingy(tokens, i, errorStr);
                    if(newNode) {
                        newNode->setName(name);
                        node->addChildToEnd(newNode);
                    }
                }

            } else {

                ERROR("Unknown token while parsing object");
                i++;

            }

        }

        // Skip closing curly.
        i++;

        return node;
    }

    static ParserNode *parseJsonArray(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr) {

        // Expect and skip opening bracket.
        assert(TOKEN_TYPE(0) == JSPT_LEFTBRACKET);
        i++;

        ParserNode *node = new ParserNode();

        while(
            i < tokens.size() &&
            TOKEN_TYPE(0) != JSPT_RIGHTBRACKET) {

            if(TOKEN_TYPE(0) == JSPT_COMMA) {

                // Skip commas.
                i++;

            } else if(TOKEN_TYPE(0) == JSPT_STRING) {

                // Random strings stuck in arrays.
                ParserNode *arrayStringElement = new ParserNode();
                arrayStringElement->setName("_arrayObject");
                arrayStringElement->setStringValue(
                    "_string", tokens[i]->str);
                node->addChildToEnd(arrayStringElement);
                i++;

            } else {

                ParserNode *objectNode =
                    parseJsonThingy(tokens, i, errorStr);

                node->addChildToEnd(objectNode);
            }
        }

        // Skip end bracket.
        i++;

        return node;
    }

    ParserNode *parseJsonString(
        const std::string &str,
        std::string *errorStr) {

        // Place to store errors if we don't care about them.
        string junk;

        // Tokenize it.
        vector<ParserToken*> tokens;
        tokenizeJson(str, errorStr ? *errorStr : junk, tokens);
        if(errorStr && errorStr->size()) {
            return NULL;
        }

        // Parse it.
        unsigned int i = 0;
        ParserNode *node = parseJsonThingy(tokens, i, errorStr ? *errorStr : junk);

        // Clean up.
        for(unsigned int i = 0; i < tokens.size(); i++) {
            delete tokens[i];
        }

        if(!errorStr && junk.size()) {
            cout << "JSON parse errors:" << endl;
            cout << junk << endl;
        }

        return node;

    }

    // ----------------------------------------------------------------------
    //  Output related stuff
    // ----------------------------------------------------------------------

    // Different from the normal output indent!
    static void indentOutputJson(ostream &out, int indentLevel) {
        for(int i = 0; i < indentLevel; i++) {
            out << "  ";
        }
    }

    static bool nodeIsArray(const ParserNode *node) {
        if(!node->getNumChildren()) return false;

        // If it has any values, then it's not an array.
        vector<string> valueNames;
        node->getValueNames(valueNames);
        if(valueNames.size())
            return false;

        // If it has anything named something other than "null", then
        // it's not an array.
        for(int i = 0; i < node->getNumChildren(); i++) {

            const ParserNode *childNode = node->getChild(i);
            string childName = childNode->getName();

            if(childNode && (
                   childName != "null" &&
                   childName != "_null" &&
                   childName != "_arrayObject")) {

                return false;
            }
        }

        return true;
    }

    static void printJsonValue(const std::string &value, std::ostream &out) {

        if(value == "true") {

            out << "true";

        } else if(value == "false") {

            out << "false";

        } else {

            // FIXME: Number detection doesn't know how to recognize
            // nan, inf, undef, and probably a bunch of other stuff.
            bool isNumber = true;
            if(value.size()) {
                bool usedDecimal = false;
                bool usedE = false;
                bool justUsedE = false;
                for(unsigned int i = 0; i < value.size(); i++) {

                    bool usedELast = justUsedE;
                    justUsedE = false;

                    if(value[i] >= '0' && value[i] <= '9') {

                        // These are fine.

                    } else if(value[i] == '.') {

                        // Decimal place. Once per number.
                        if(usedDecimal) {
                            isNumber = false;
                            break;
                        }

                        usedDecimal = true;

                    } else if(value[i] == '-' && i != 0 && !usedELast) {

                        // Negative sign. Can only come at the start or
                        // after 'e'.
                        isNumber = false;
                        break;

                    } else if(value[i] == 'e') {

                        // 'e' in floating point. Probably needs to come
                        // before and after a number. Can only have one
                        // per number.
                        if(usedE || i == 0 || i == value.size() - 1) {
                            isNumber = false;
                            break;
                        }

                        // Can't use decimals anymore.
                        usedDecimal = true;

                        justUsedE = true;
                        usedE = true;

                    } else {

                        isNumber = false;
                        break;
                    }
                }

            } else {

                // Empty string.
                isNumber = false;
            }

            if(isNumber) {
                out << value;
            } else {
                out << "\"" << stringEscape(value) << "\"";
            }

        }


    }

    void ParserNode::outputJson(
        std::ostream &out,
        int indentLevel,
        bool lineStart) const {

        if(lineStart) {
            indentOutputJson(out, indentLevel);
        }

        if(name == "_arrayObject") {
            printJsonValue(
                getStringValueDirty("_string"),
                out);
            return;
        }

        if(name == "_null") {
            out << "null";
            return;
        }

        bool isArray = nodeIsArray(this);

        out << (isArray ? "[" : "{");

        if(values.size() || getNumChildren()) {
            cout << endl;
        }

        // Print out attributes.
        if(!isArray) {
            for(std::map<std::string, std::string>::const_iterator i = values.begin();
                i != values.end(); i++) {

                indentOutputJson(out, indentLevel + 1);
                out << "\"" << stringEscape((*i).first) << "\": ";
                printJsonValue((*i).second, out);

                // Wonky way to see if we're almost at the end.
                std::map<std::string, std::string>::const_iterator j = i;
                j++;
                if(j != values.end() || getNumChildren()) {
                    out << "," << endl;
                }
            }
        }

        // Print out children.
        for(int i = 0; i < getNumChildren(); i++) {
            indentOutputJson(out, indentLevel + 1);

            if(!isArray) {
                out << "\"" << stringEscape(children[i]->name) << "\": ";
            }

            children[i]->outputJson(out, indentLevel + 1, false);

            if(i + 1 < getNumChildren()) {
                out << "," << endl;
            }
        }

        out << (isArray ? "]" : "}");

        if(indentLevel == 0) {
            out << endl;
        }
    }


}
