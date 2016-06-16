// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
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

// JSON parser module for lilyparser.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "malstring.h"
#include "filesystem.h"
#include "lilyparser.h"

#include <cassert>
#include <string>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Parse a JSON string into a ParserNode tree. Some elements
    /// don't translate very well, so "_arrayObject" is used as the
    /// name for individual entries in arrays, and "_string" for
    /// strings inside arrays.
    inline ParserNode *parseJsonString(
        const std::string &str,
        std::string *errorStr);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
  #define EXPOP_JSON_TOKEN_TYPE(x) (parserJsonGetTokenTypeSafe(tokens, i + (x)))
  #define EXPOP_JSON_LINE_NUMBER (parserJsonGetTokenLineSafe(tokens, i))
  #define EXPOP_JSON_ERROR(x)                                   \
    do {                                                        \
        std::ostringstream ostr;                                \
        ostr << EXPOP_JSON_LINE_NUMBER << ": " << (x) << endl;  \
        errorStr = errorStr + ostr.str();                       \
    } while(0);

    //  Tokenization

    enum JsonParserTokenType
    {
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

    inline std::string parserJsonCharToString(char c)
    {
        char s1[2] = { c, 0 };
        return string(s1);
    }

    inline bool parserJsonIsNumber(char c)
    {
        if(c >= '0' && c <= '9') return true;
        if(c == '-' || c == '.') return true;
        return false;
    }

    inline void parserJsonTokenizeJson(
        const std::string &str,
        std::string &errorStr,
        vector<ParserToken*> &tokens)
    {
        unsigned int ptr = 0;
        unsigned int lineNumber = 1;

        while(ptr < str.size()) {

            char c = str[ptr];

            if(c == '{' || c == '}') {

                tokens.push_back(
                    new ParserToken(
                        parserJsonCharToString(c),
                        c == '{' ? JSPT_OPENCURLY : JSPT_CLOSECURLY,
                        lineNumber));

            } else if(c == '[' || c == ']') {

                tokens.push_back(
                    new ParserToken(
                        parserJsonCharToString(c),
                        c == '[' ? JSPT_LEFTBRACKET : JSPT_RIGHTBRACKET,
                        lineNumber));

            } else if(c == ':') {

                tokens.push_back(
                    new ParserToken(
                        parserJsonCharToString(c),
                        JSPT_COLON,
                        lineNumber));

            } else if(c == ',') {

                tokens.push_back(
                    new ParserToken(
                        parserJsonCharToString(c),
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

            } else if(parserJsonIsNumber(c)) {

                // FIXME: This is too generous with anything
                // containing 'e'. It will not spit out an error when
                // it should.
                unsigned int startPt = ptr;

                // Keep going as long as we're seeing number-like
                // things.
                while(ptr < str.size() && (parserJsonIsNumber(str[ptr]) || str[ptr] == 'e')) {
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

    inline int parserJsonGetTokenTypeSafe(
        const std::vector<ParserToken*> &tokens,
        unsigned int i)
    {
        if(i >= tokens.size()) {
            return JSPT_UNKNOWN;
        }
        return tokens[i]->type;
    }

    inline unsigned int parserJsonGetTokenLineSafe(
        const vector<ParserToken*> &tokens,
        unsigned int i)
    {
        if(!tokens.size())
            return 0;

        if(i >= tokens.size()) {
            return tokens[tokens.size() - 1]->lineNumber;
        }

        return tokens[i]->lineNumber;
    }

    // Parsing

    inline ParserNode *parserJsonParseJsonObject(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr);

    inline ParserNode *parserJsonParseJsonArray(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr);

    inline ParserNode *parserJsonParseJsonThingy(
        vector<ParserToken*> &tokens,
        unsigned int &i,
        string &errorStr)
    {
        if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_OPENCURLY) {
            return parserJsonParseJsonObject(tokens, i, errorStr);
        } else if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_LEFTBRACKET) {
            return parserJsonParseJsonArray(tokens, i, errorStr);
        } else if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_NULL) {
            ParserNode *newNode = new ParserNode();
            newNode->setName("_null");
            i++;
            return newNode;
        } else {
            EXPOP_JSON_ERROR("Unknown token type");
            i++;
            return new ParserNode();
        }
    }

    inline ParserNode *parserJsonParseJsonObject(
        std::vector<ParserToken*> &tokens,
        unsigned int &i,
        std::string &errorStr)
    {
        // Expect and skip opening curly brace.
        assert(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_OPENCURLY);
        i++;

        ParserNode *node = new ParserNode();

        while(
            i < tokens.size() &&
            EXPOP_JSON_TOKEN_TYPE(0) != JSPT_CLOSECURLY) {

            if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_COMMA) {

                // Skip commas.
                i++;

            } else if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_STRING &&
                      EXPOP_JSON_TOKEN_TYPE(1) == JSPT_COLON) {

                // Objects and attributes.

                std::string name = tokens[i]->str;
                i += 2;

                if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_STRING) {

                    node->setStringValue(
                        name,
                        tokens[i]->str);
                    i++;

                } else {

                    ParserNode *newNode =
                        parserJsonParseJsonThingy(tokens, i, errorStr);
                    if(newNode) {
                        newNode->setName(name);
                        node->addChildToEnd(newNode);
                    }
                }

            } else {

                EXPOP_JSON_ERROR("Unknown token while parsing object");
                i++;

            }

        }

        // Skip closing curly.
        i++;

        return node;
    }

    inline ParserNode *parserJsonParseJsonArray(
        std::vector<ParserToken*> &tokens,
        unsigned int &i,
        std::string &errorStr)
    {

        // Expect and skip opening bracket.
        assert(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_LEFTBRACKET);
        i++;

        ParserNode *node = new ParserNode();

        while(
            i < tokens.size() &&
            EXPOP_JSON_TOKEN_TYPE(0) != JSPT_RIGHTBRACKET) {

            if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_COMMA) {

                // Skip commas.
                i++;

            } else if(EXPOP_JSON_TOKEN_TYPE(0) == JSPT_STRING) {

                // Random strings stuck in arrays.
                ParserNode *arrayStringElement = new ParserNode();
                arrayStringElement->setName("_arrayObject");
                arrayStringElement->setStringValue(
                    "_string", tokens[i]->str);
                node->addChildToEnd(arrayStringElement);
                i++;

            } else {

                ParserNode *objectNode =
                    parserJsonParseJsonThingy(tokens, i, errorStr);

                node->addChildToEnd(objectNode);
            }
        }

        // Skip end bracket.
        i++;

        return node;
    }

    inline ParserNode *parseJsonString(
        const std::string &str,
        std::string *errorStr)
    {
        // Place to store errors if we don't care about them.
        std::string junk;

        // Tokenize it.
        std::vector<ParserToken*> tokens;
        parserJsonTokenizeJson(str, errorStr ? *errorStr : junk, tokens);
        if(errorStr && errorStr->size()) {
            return NULL;
        }

        // Parse it.
        unsigned int i = 0;
        ParserNode *node = parserJsonParseJsonThingy(tokens, i, errorStr ? *errorStr : junk);

        // Clean up.
        for(unsigned int i = 0; i < tokens.size(); i++) {
            delete tokens[i];
        }

        return node;
    }

    // Output related stuff


    // Different from the normal output indent!
    inline void parserJsonIndentOutputJson(ostream &out, int indentLevel)
    {
        for(int i = 0; i < indentLevel; i++) {
            out << "  ";
        }
    }

    inline bool parserJsonNodeIsArray(const ParserNode *node)
    {
        if(!node->getNumChildren()) return false;

        // If it has any values, then it's not an array.
        std::vector<std::string> valueNames;
        node->getValueNames(valueNames);
        if(valueNames.size())
            return false;

        // If it has anything named something other than "null", then
        // it's not an array.
        for(int i = 0; i < node->getNumChildren(); i++) {

            const ParserNode *childNode = node->getChild(i);
            std::string childName = childNode->getName();

            if(childNode && (
                   childName != "null" &&
                   childName != "_null" &&
                   childName != "_arrayObject")) {

                return false;
            }
        }

        return true;
    }

    inline void parserJsonPrintJsonValue(const std::string &value, std::ostream &out)
    {
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

    inline void ParserNode::outputJson(
        std::ostream &out,
        int indentLevel,
        bool lineStart) const
    {
        if(lineStart) {
            parserJsonIndentOutputJson(out, indentLevel);
        }

        if(name == "_arrayObject") {
            parserJsonPrintJsonValue(
                getStringValueDirty("_string"),
                out);
            return;
        }

        if(name == "_null") {
            out << "null";
            return;
        }

        bool isArray = parserJsonNodeIsArray(this);

        out << (isArray ? "[" : "{");

        if(values.size() || getNumChildren()) {
            out << endl;
        }

        // Print out attributes.
        if(!isArray) {
            for(std::map<std::string, std::string>::const_iterator i = values.begin();
                i != values.end(); i++) {

                parserJsonIndentOutputJson(out, indentLevel + 1);
                out << "\"" << stringEscape((*i).first) << "\": ";
                parserJsonPrintJsonValue((*i).second, out);

                // Wonky way to see if we're almost at the end.
                std::map<std::string, std::string>::const_iterator j = i;
                j++;
                if(j != values.end() || getNumChildren()) {
                    out << ",";
                }

                out << std::endl;
            }
        }

        // Print out children.
        for(int i = 0; i < getNumChildren(); i++) {
            parserJsonIndentOutputJson(out, indentLevel + 1);

            if(!isArray) {
                out << "\"" << stringEscape(children[i]->name) << "\": ";
            }

            children[i]->outputJson(out, indentLevel + 1, false);

            if(i + 1 < getNumChildren()) {
                out << ",";
            }

            out << std::endl;
        }

        parserJsonIndentOutputJson(out, indentLevel);
        out << (isArray ? "]" : "}");

        if(indentLevel == 0) {
            out << std::endl;
        }
    }



}

