// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
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

// XML-like parser for the Lily-Engine-Utils hierarchical data format
// module.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iostream>
#include <sstream>
#include <cstdarg>

#include "malstring.h"
#include "lilyparser.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Parse a string of XML stuff. Not fully standards compliant.
    /// Probably not even mostly standards compliant. Actually, it's
    /// more just an XML-like language. This will add an extra root
    /// node to contain multiple root nodes in a possibly-badly formed
    /// XML file. So the output of outputXml(), re-parsed into this,
    /// will be different, with an extra level added.
    ParserNode *parseXmlString(const std::string &str, std::string *errorStr = NULL);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    enum XmlTokenType
    {
        XMLTOKEN_EQUALS,
        XMLTOKEN_LT,
        XMLTOKEN_GT,
        XMLTOKEN_SLASH,
        XMLTOKEN_IDENTIFIER,
        XMLTOKEN_QUOTEDSTRING,
        XMLTOKEN_BANG,
        XMLTOKEN_MINUS,
        XMLTOKEN_QUESTION,
        XMLTOKEN_WHITESPACE,
        XMLTOKEN_WHAT,
        XMLTOKEN_TEXT,
        XMLTOKEN_COMMENTSTART,
        XMLTOKEN_COMMENTEND,
        XMLTOKEN_CDATA
    };

    class XmlToken
    {
    public:
        std::string str;
        XmlTokenType type;
        unsigned int lineNumber;

        inline XmlToken(const std::string &inStr, XmlTokenType type, int lineNumber)
        {
            this->str = inStr;
            this->type = type;
            this->lineNumber = lineNumber;
        }
    };

    // Start this one character in from the quote mark.
    inline void parserXmlReadQuotedString(
        const std::string &str, unsigned int &pos,
        std::string &outStr, unsigned int &lineNumber)
    {
        bool ignoreNext = false;
        unsigned int startPos = pos;

        while(pos < str.size() && (str[pos] != '"' || ignoreNext)) {

            if(str[pos] == '\\' && !ignoreNext) {
                ignoreNext = true;
            } else {
                ignoreNext = false;
            }

            if(str[pos] == '\n') lineNumber++;

            pos++;
        }

        outStr = str.substr(startPos, pos - startPos);
    }

    inline bool parserXmlIsValidIdentifierCharacter(char c)
    {
        if(c >= 'a' && c <= 'z') return true;
        if(c >= 'A' && c <= 'Z') return true;
        if(c >= '0' && c <= '9') return true;
        if(c == ':' || c == '_' || c == '-') return true;

        // All of Unicode is now a valid identifier. Hurray!
        if(c & 0x10000000) return true;

        return false;
    }

    inline void parserXmlReadIdentifier(
        const std::string &str, unsigned int &pos,
        std::string &outStr)
    {
        unsigned int startPos = pos;

        while(pos < str.size()) {
            if(!parserXmlIsValidIdentifierCharacter(str[pos])) {
                break;
            }
            pos++;
        }

        outStr = str.substr(startPos, pos - startPos);

        // Leave us pointing at the last character instead of the
        // character after the end (because pos++ will happen in the main
        // block at the end of the loop iteration).
        pos--;
    }

    inline bool parserXmlReadAheadTokenTypes(
        std::vector<XmlToken*> &tokens,
        unsigned int start,
        unsigned int count,
        ...)
    {
        if(tokens.size() <= start + count - 1) return false;

        va_list argList;
        va_start(argList, count);

        // FIXME: MinGW build complains about unsigned/signed
        // comparison here.
        while(count && tokens[start]->type == va_arg(argList, unsigned int)) {
            start++;
            count--;
        }

        va_end(argList);

        if(!count) return true;

        return false;
    }

    inline void parserXmlParseTag(
        std::vector<XmlToken*> &tokens,
        unsigned int &tokNum,
        bool &selfTerminate,
        ParserNode *node,
        std::ostringstream &errorOut)
    {
        // Read in all attributes.
        while(parserXmlReadAheadTokenTypes(
                  tokens, tokNum, 3,
                  XMLTOKEN_IDENTIFIER,
                  XMLTOKEN_EQUALS,
                  XMLTOKEN_QUOTEDSTRING)) {

            std::string attributeName = tokens[tokNum]->str;
            std::string attributeValue = tokens[tokNum+2]->str;

            node->setStringValue(attributeName, attributeValue);

            tokNum += 3;
        }

        // Look for self-terminating stuff.
        if(parserXmlReadAheadTokenTypes(
               tokens, tokNum, 1,
               XMLTOKEN_SLASH))
        {
            // This tag is self-terminating.
            selfTerminate = true;
            tokNum++;
        }

        // Look for the end of the tag.
        if(!parserXmlReadAheadTokenTypes(
                tokens, tokNum, 1,
                XMLTOKEN_GT))
        {
            if(tokNum < tokens.size()) {
                errorOut << tokens[tokNum]->lineNumber <<
                    ": Parse error at \"" << tokens[tokNum]->str << "\"." << std::endl;
            } else {
                errorOut << tokens[tokNum]->lineNumber <<
                    ": XML ended in the middle of a tag." << std::endl;
            }
        } else {
            // Skip the end '>'.
            tokNum++;
        }
    }

    inline ParserNode *parseXmlTokens(
        std::vector<XmlToken*> &tokens,
        unsigned int &tokNum,
        std::ostringstream &errorOut)
    {
        // Make the root node and a stack of nodes.
        ParserNode *rootNode = new ParserNode("_root");
        std::vector<ParserNode *> nodeStack;
        nodeStack.push_back(rootNode);

        while(tokNum < tokens.size()) {

            if(tokens[tokNum]->type == XMLTOKEN_TEXT ||
               tokens[tokNum]->type == XMLTOKEN_CDATA) {

                std::ostringstream finalStrOut;

                while(tokens[tokNum]->type == XMLTOKEN_TEXT ||
                      tokens[tokNum]->type == XMLTOKEN_CDATA) {

                    std::string tokStr;
                    if(tokens[tokNum]->type == XMLTOKEN_TEXT) {
                        tokStr = stringTrim(tokens[tokNum]->str);
                    } else {
                        tokStr = tokens[tokNum]->str;
                    }

                    finalStrOut << tokStr;

                    tokNum++;
                }

                if(finalStrOut.str().size()) {
                    ParserNode *newNode = new ParserNode("_text");
                    newNode->setStringValue("text", finalStrOut.str());
                    nodeStack[nodeStack.size()-1]->addChildToEnd(newNode);
                }

            } else if(parserXmlReadAheadTokenTypes(
                          tokens, tokNum, 2,
                          XMLTOKEN_LT,
                          XMLTOKEN_IDENTIFIER))
            {

                tokNum += 2;

                // TODO: Check to make sure this is a valid tag name and
                // doesn't clobber any of our built-in stuff.

                ParserNode *newNode = new ParserNode(tokens[tokNum-1]->str);

                bool selfTerminate = false;
                parserXmlParseTag(tokens, tokNum, selfTerminate, newNode, errorOut);

                nodeStack[nodeStack.size()-1]->addChildToEnd(newNode);
                if(!selfTerminate) {
                    nodeStack.push_back(newNode);
                }

            } else if(parserXmlReadAheadTokenTypes(
                          tokens, tokNum, 4,
                          XMLTOKEN_LT,
                          XMLTOKEN_SLASH,
                          XMLTOKEN_IDENTIFIER,
                          XMLTOKEN_GT)) {

                tokNum += 4;

                // TODO: Check to make sure this is a valid tag name and
                // doesn't clobber any of our built-in stuff.

                if(nodeStack.size() > 1 && nodeStack[nodeStack.size()-1]->getName() == tokens[tokNum-2]->str) {

                    // Pop the stack of nodes.
                    nodeStack.erase(nodeStack.end() - 1);

                } else {

                    errorOut << tokens[tokNum-2]->lineNumber <<
                        ": Mismatched tags: " << nodeStack[nodeStack.size()-1]->getName() <<
                        " and " << tokens[tokNum-2]->str << std::endl;

                }

            } else {

                errorOut << tokens[tokNum]->lineNumber << ": Parse error at \"" << tokens[tokNum]->str << "\"." << std::endl;
                tokNum++;
            }

        }

        return rootNode;
    }

    inline ParserNode *parseXmlString(
        const std::string &str,
        std::string *errorStr)
    {
        std::ostringstream errorOut;

        // First, tokenize.

        unsigned int pos = 0;
        std::vector<XmlToken*> tokens;
        std::string strTmp;
        bool inTag = false;
        std::ostringstream textStr;
        unsigned int lineNumber = 1;

        while(pos < str.size()) {

            if(!inTag) {

                // We are not in a tag. Look for a tag start, or just keep
                // processing normal text.

                if(pos + 3 < str.size() &&
                   str[pos] == '<' &&
                   str[pos+1] == '!' &&
                   str[pos+2] == '-' &&
                   str[pos+3] == '-')
                {
                    // This is a comment. Just skip past it.  Read until
                    // we see the "-->".

                    pos += 4;

                    while(pos + 1 < str.size()) {

                        if(str[pos] == '\n') lineNumber++;

                        if(str[pos] == '-' &&
                           str[pos+1] == '-' &&
                           str[pos+2] == '>') {

                            break;
                        }

                        pos++;
                    }

                    pos += 2;

                } else if(pos + 9 < str.size() && str.substr(pos, 9) == "<![CDATA[") {

                    // CDATA block. Just read until we see the end of
                    // the block. Ignore everything in it except for
                    // line numbers.

                    pos += 9;

                    unsigned int cdataStart = pos;
                    unsigned int cdataEnd = cdataStart;

                    while(1) {

                        if(pos + 3 < str.size() && str.substr(pos, 3) == "]]>") {
                            // Found the end.
                            pos += 2;
                            break;
                        }

                        if(pos >= str.size()) break;

                        if(str[pos] == '\n') lineNumber++;
                        pos++;
                        cdataEnd = pos;
                    }

                    // We're not going to concatenate this with the
                    // block on the end of the list. Handle that later
                    // in parsing and just glob all adjacent text and
                    // CDATA blocks together.
                    tokens.push_back(
                        new XmlToken(
                            str.substr(cdataStart, cdataEnd - cdataStart),
                            XMLTOKEN_CDATA,
                            lineNumber));
                    textStr.str("");

                } else if(str[pos] == '<') {

                    // Add all the text that we've been accumulating while
                    // not in a tag.
                    if(textStr.str().size()) {
                        tokens.push_back(
                            new XmlToken(
                                stringXmlUnescape(textStr.str()),
                                XMLTOKEN_TEXT,
                                lineNumber));
                        textStr.str("");
                    }

                    // Tag start.
                    inTag = true;
                    tokens.push_back(new XmlToken("<", XMLTOKEN_LT, lineNumber));

                } else {

                    if(str[pos] == '\n') lineNumber++;
                    textStr << str[pos];

                }

            } else {

                // We are inside a tag.

                // Skip whitespace.
                while(pos < str.size() && isWhiteSpace(str[pos])) {
                    if(str[pos] == '\n') lineNumber++;
                    pos++;
                }
                if(pos >= str.size()) break;

                if(str[pos] == '\n') lineNumber++;

                if(str[pos] == '>') {

                    // Tag end.
                    inTag = false;
                    tokens.push_back(new XmlToken(">", XMLTOKEN_GT, lineNumber));

                } else if(str[pos] == '"') {

                    // Quoted string.
                    pos++;
                    parserXmlReadQuotedString(str, pos, strTmp, lineNumber);
                    tokens.push_back(new XmlToken(stringXmlUnescape(strTmp), XMLTOKEN_QUOTEDSTRING, lineNumber));

                } else if(str[pos] == '=') {

                    // Equals.
                    tokens.push_back(new XmlToken("=", XMLTOKEN_EQUALS, lineNumber));

                } else if(str[pos] == '/') {

                    // Slash.
                    tokens.push_back(new XmlToken("/", XMLTOKEN_SLASH, lineNumber));

                } else if(parserXmlIsValidIdentifierCharacter(str[pos])) {

                    // Normal identifier.
                    parserXmlReadIdentifier(str, pos, strTmp);
                    tokens.push_back(new XmlToken(strTmp, XMLTOKEN_IDENTIFIER, lineNumber));

                } else if(str[pos] == '!') {

                    // "!"
                    tokens.push_back(new XmlToken("!", XMLTOKEN_BANG, lineNumber));

                } else if(str[pos] == '?') {

                    // "?"
                    tokens.push_back(new XmlToken("?", XMLTOKEN_QUESTION, lineNumber));

                } else if(str[pos] == '-') {

                    // "-"
                    tokens.push_back(new XmlToken("-", XMLTOKEN_MINUS, lineNumber));

                } else {

                    // Unknown token type?
                    tokens.push_back(new XmlToken(str.substr(pos, 1), XMLTOKEN_WHAT, lineNumber));

                }

            }

            pos++;
        }

        // // Temp debug output - spit out the list of tokens.
        // for(unsigned int i = 0; i < tokens.size(); i++) {
        //     cout << setw(4) << tokens[i]->lineNumber << " " << setw(4) << i << " " << setw(2) << tokens[i]->type << " " << tokens[i]->str << endl;
        // }

        unsigned int tokNum = 0;

        // Skip past any XML declarations because we just don't care about
        // them.
        while(tokNum < tokens.size() &&
              parserXmlReadAheadTokenTypes(
                  tokens, tokNum, 2,
                  XMLTOKEN_LT,
                  XMLTOKEN_QUESTION)) {

            tokNum += 2;

            while(!parserXmlReadAheadTokenTypes(
                      tokens, tokNum, 2,
                      XMLTOKEN_QUESTION,
                      XMLTOKEN_GT)) {

                tokNum++;
            }

            tokNum += 2;
        }

        // Dive right into parsing now.
        ParserNode *rootNode = parseXmlTokens(tokens, tokNum, errorOut);

        // Clean up.
        for(unsigned int i = 0; i < tokens.size(); i++) {
            delete tokens[i];
        }

        // Dump all error messages.
        if(errorStr) {
            *errorStr = errorOut.str();
        }

        return rootNode;
    }

}



