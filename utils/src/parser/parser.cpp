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

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstring>
#include <cstdlib>
using namespace std;

#include <lilyengine/malstring.h>
#include <lilyengine/filesystem.h>
#include <lilyengine/lilyparser.h>

namespace ExPop {

    // ------------------------------------------------------------------------
    //   Utility junk
    // ------------------------------------------------------------------------

    inline bool isSymbolCharacter(char c) {

        if((c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_')) {

            return true;
        }

        return false;
    }

    inline bool isNumberCharacter(char c) {

        if((c >= '0' && c <= '9') ||
           c == '.' ||
           c == '-') {

            return true;
        }

        return false;
    }

    enum ParserTokenType {
        PT_UNKNOWN,

        PT_BLOCKSTART,
        PT_BLOCKEND,

        PT_EQUALS,
        PT_LINEEND,

        PT_SYMBOL,
        PT_STRING,
        PT_NUMBER,
    };

    static void indentOutput(ostream &out, int indentLevel) {
        for(int i = 0; i < indentLevel; i++) {
            out << "    ";
        }
    }

    // ------------------------------------------------------------------------
    //   ParserNode implementation
    // ------------------------------------------------------------------------

    std::string ParserNode::getName(void) const {
        return name;
    }

    void ParserNode::setName(const std::string &name) {
        this->name = name;
    }

    ParserNode *ParserNode::getChild(int index) {
        if(index >= (int)children.size() || index < 0) {
            return NULL;
        }
        return children[index];
    }

    const ParserNode *ParserNode::getChild(int index) const {
        if(index >= (int)children.size() || index < 0) {
            return NULL;
        }
        return children[index];
    }

    int ParserNode::getNumChildren(void) const {
        return (int)children.size();
    }

    void ParserNode::addChildToEnd(ParserNode *node) {
        children.push_back(node);
    }

    void ParserNode::addChildToStart(ParserNode *node) {
        children.insert(children.begin(), node);
    }

    void ParserNode::output(ostream &out, int indentLevel) const {

        // Print all values.
        bool atLeastOneValue = false;
        for(std::map<std::string, std::string>::const_iterator i = values.begin(); i != values.end(); i++) {
            indentOutput(out, indentLevel);

            string escapedOut = stringEscape((*i).second);

            // 80 columns, minus...
            //   indentation spaces
            //   spaces on either side of "="
            //   '='
            //   ';'
            //   quotes for the string
            //   the name of the value
            int maxLength = 80 - ((indentLevel * 4) + 6 + (*i).first.size());

            if((int)escapedOut.size() > maxLength) {

                // This line is too long. We need to break this up.

                // First line is just "name = "
                out << (*i).first << " = " << endl;

                string indented;
                for(int j = 0; j < indentLevel + 1; j++) {
                    indented = indented + "    ";
                }

                escapedOut = stringEscape(
                    // (*i).second, "\\n\"\n" + indented + "\"");
                    (*i).second, true);

                out << indented << "\"" << escapedOut << "\";" << endl;

            } else {

                // We can fit this on one line.
                out << (*i).first << " = \"" << escapedOut << "\";" << endl;
            }

            atLeastOneValue = true;
        }

        // Add some space before the children.
        if(atLeastOneValue && children.size()) {
            out << endl;
        }

        // Print all children.
        for(unsigned int cn = 0; cn < children.size(); cn++) {

            indentOutput(out, indentLevel);

            if(children[cn]->name.size()) {
                out << children[cn]->name << " ";
            } else {
                out << "null " << endl;
            }

            out << "{" << endl;

            children[cn]->output(out, indentLevel + 1);

            indentOutput(out, indentLevel);
            out << "}" << endl;

            // Add a line between children.
            if(cn + 1 != children.size()) {
                out << endl;
            }
        }
    }

    static std::string escapeCDATA(const std::string &str) {

        ostringstream outStr;

        unsigned int pos = 0;

        while(pos < str.size()) {

            if(pos + 3 < str.size() &&
               str[pos] == ']' &&
               str[pos+1] == ']' &&
               str[pos+2] == '>') {

                outStr << "]]]><![CDATA[]>";
                pos += 3;

            } else {

                outStr << str[pos];
                pos++;
            }

        }

        return outStr.str();

    }

    void ParserNode::outputXml(std::ostream &out, int indentLevel) const {

        indentOutput(out, indentLevel);

        bool headerOnly = false;
        if(getName() == "_root") {
            headerOnly = true;
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;

            // This will actually wrap around to 0xFFFFFFFF or
            // somesuch, but whatever.
            indentLevel--;
        }

        if(getName() == "_text") {

            string outStr;

            // Check for leading or trailing whitespace so we know if
            // we need CDATA.
            bool useCDATA = false;
            string textStr = getStringValueDirty("text");
            if(textStr.size() && (isWhiteSpace(textStr[0]) || isWhiteSpace(textStr[textStr.size() - 1]))) {
                useCDATA = true;
            }

            if(useCDATA) {
                out << "<![CDATA[" << escapeCDATA(textStr) << "]]>" << endl;
            } else {
                out << stringXmlEscape(textStr) << endl;
            }

        } else {

            bool hadAttributes = false;

            if(!headerOnly) {

                out << "<" << getName();

                // Output attributes.
                bool firstAttrib = false;
                for(map<string, string>::const_iterator i = values.begin(); i != values.end(); i++) {

                    if(firstAttrib) {
                        firstAttrib = false;
                    } else {
                        out << " ";
                    }

                    out << i->first << "=\"" << stringXmlEscape(i->second) << "\"";
                    hadAttributes = true;
                }
            }

            // Output children.
            if(getNumChildren()) {

                // Finish the starting tag.
                if(!headerOnly) {
                    out << ">" << endl;
                }

                for(int i = 0; i < getNumChildren(); i++) {
                    children[i]->outputXml(out, indentLevel + 1);
                }

                indentOutput(out, indentLevel);

                // End tag.
                if(!headerOnly) {
                    out << "</" << getName() << ">" << endl;
                }

            } else {

                if(!headerOnly) {
                    out << (hadAttributes ? " " : "") << "/>" << endl;
                }
            }
        }
    }

    ParserNode::~ParserNode(void) {
        for(unsigned int cn = 0; cn < children.size(); cn++) {
            delete children[cn];
        }
    }

    std::string ParserNode::toString(void) const {
        ostringstream str;
        output(str, 0);
        return str.str();
    }


    bool ParserNode::getBooleanValue(const std::string &name, bool *value) const {

        if(values.count(name)) {

            string val = values.find(name)->second;

            if(val == "false" || val == "0" || val == "") {
                *value = false;
            } else {
                *value = true;
            }

            return true;
        }

        return false;
    }

    bool ParserNode::getStringValue(const std::string &name, std::string *value) const {

        if(values.count(name)) {

            string val = values.find(name)->second;
            *value = val;
            return true;
        }

        return false;
    }

    std::string ParserNode::getStringValueDirty(const std::string &name) const {
        string str = "";
        getStringValue(name, &str);
        return str;
    }

    bool ParserNode::getFloatValue(const std::string &name, float *value) const {

        if(values.count(name)) {

            string val = values.find(name)->second;
            *value = atof(val.c_str());
            return true;
        }

        return false;
    }

    bool ParserNode::getIntValue(const std::string &name, int *value) const {

        if(values.count(name)) {

            string val = values.find(name)->second;
            *value = atoi(val.c_str());
            return true;
        }

        return false;
    }

    bool ParserNode::getBinaryValue(const std::string &name, char **buf, int *length) const {

        if(values.count(name)) {

            *buf = strDecodeHex(values.find(name)->second, length);

            // // TEMPORARY
            // int j = 0;
            // for(int i = 0; i < *length; i++) {
            //     j += (*buf)[i];
            // }
            // cout << "Checksum: " << j << endl;

            return true;
        }

        return false;
    }

    void ParserNode::setBinaryValue(const std::string &name, const char *buf, int length) {
        strEncodeHex(buf, length, values[name], 40);

        // int j = 0;
        // for(int i = 0; i < length; i++) {
        //     j += buf[i];
        // }
        // cout << "Checksum: " << j << endl;
    }

    void ParserNode::setBooleanValue(const std::string &name, bool value) {
        ostringstream str;
        str << (value ? "true" : "false");
        values[name] = str.str();
    }

    void ParserNode::setStringValue(const std::string &name, const std::string &value) {
        values[name] = value;
    }

    void ParserNode::setFloatValue(const std::string &name, float value) {
        ostringstream str;
        str << value;
        values[name] = str.str();
    }

    void ParserNode::setIntValue(const std::string &name, int value) {
        ostringstream str;
        str << value;
        values[name] = str.str();
    }

    void ParserNode::clearValue(const std::string &name) {

        values.erase(name);

    }

    void ParserNode::getValueNames(std::vector<std::string> &names) const {
        for(std::map<std::string, std::string>::const_iterator i = values.begin();
            i != values.end(); i++) {
            names.push_back((*i).first);
        }
    }

    int ParserNode::getChildIndexByName(const std::string &name, int after) {

        if(after < -1 || after >= int(children.size()) - 1) {
            return int(children.size());
        }

        for(int i = after + 1; i < int(children.size()); i++) {
            if(children[i] && children[i]->name == name) {
                return i;
            }
        }

        return int(children.size());
    }

    ParserNode *ParserNode::getChildByName(const std::string &name) {

        return getChild(getChildIndexByName(name));

    }

    ostream &operator<<(ostream &out, const ParserNode &node) {

        node.output(out, 0);

        return out;
    }

    ParserNode::ParserNode(void) {
        name = "null";
    }

    ParserNode::ParserNode(const std::string &name) {
        this->name = name;
    }


    // ------------------------------------------------------------------------
    //   The parser itself
    // ------------------------------------------------------------------------

    ParserToken::ParserToken(const std::string &str, int type, int lineNumber) {
        this->str = str;
        this->type = type;
        this->lineNumber = lineNumber;
    }

    ParserToken::ParserToken(char c, int lineNumber) {

        // This seems oddly the simplest way to do this...
        str = " ";
        str[0] = c;

        this->type = PT_UNKNOWN;

        this->lineNumber = lineNumber;
    }


    static ParserNode *parseTokens(const std::vector<ParserToken*> &tokens, const std::string &nodeName, int *pos, std::string *errorStr) {

        ParserNode *node = new ParserNode();

        node->setName(nodeName);

        while(*pos < (int)tokens.size()) {

            int tokensLeft = (int)tokens.size() - *pos;

            if(tokensLeft >= 4 &&
               tokens[*pos    ]->type == PT_SYMBOL &&
               tokens[*pos + 1]->type == PT_EQUALS &&
               (   tokens[*pos + 2]->type == PT_SYMBOL ||
                   tokens[*pos + 2]->type == PT_NUMBER ||
                   tokens[*pos + 2]->type == PT_STRING) &&
               tokens[*pos + 3]->type == PT_LINEEND) {

                // Something in the form of: symbol = "someValue";

                node->setStringValue(tokens[*pos]->str, tokens[*pos + 2]->str);

                (*pos) += 4;

            } else if(tokensLeft >=2 &&
                      tokens[*pos]->type == PT_SYMBOL &&
                      tokens[*pos + 1]->type == PT_LINEEND) {

                // Simple boolean flag.

                node->setBooleanValue(tokens[*pos]->str, true);

                (*pos) += 2;

            } else if(tokensLeft >= 2 &&
                      tokens[*pos]->type == PT_SYMBOL &&
                      tokens[*pos + 1]->type == PT_BLOCKSTART) {

                // Recurse into a child thinger.

                string childNodeName = tokens[*pos]->str;

                (*pos) += 2;

                ParserNode *childNode = parseTokens(tokens, childNodeName, pos, errorStr);

                if(childNode) {

                    node->addChildToEnd(childNode);

                } else {

                    // An error happened when parsing the child. Clean
                    // up and return.
                    delete node;
                    return NULL;
                }

            } else if(tokensLeft >= 1 && tokens[*pos]->type == PT_BLOCKEND) {

                // Done with this block!
                (*pos)++;

                break;

            } else {

                // I don't know what this is. Toss out an error message
                // and bail out.
                if(errorStr) {
                    ostringstream errStrStr;
                    errStrStr << "Syntax error at: \"" << tokens[*pos]->str << "\" on line " << tokens[*pos]->lineNumber;
                    *errorStr = errStrStr.str();
                }

                delete node;
                return NULL;

            }

        }

        return node;
    }

    ParserNode *parseBuffer(const char *buf, int length, std::string *errorStr) {

        // First, tokenize.

        vector<ParserToken*> tokens;

        int pos = 0;
        int lineNumber = 0;

        while(pos < length) {

            char c = buf[pos];

            if(isWhiteSpace(c)) {

                if(c == '\n') {
                    lineNumber++;
                }

                // Just whitespace. Ignore it.
                pos++;

            } else if(
                (c == '#') ||                                       // Bash comment
                (c == '/' && pos + 1 < length && buf[pos+1] == '/') // C++ comment
                ) {

                // Comment. Read until end of line or file.

                while(pos < length && c != '\n') {
                    pos++;
                    c = buf[pos];
                }

                // Note: Don't skip over the \n. Let the line number
                // counter get it next pass.

            } else if(isNumberCharacter(c)) {

                // Number. Go to end of number.

                int numberStartPos = pos;

                while(pos < length && isNumberCharacter(c)) {
                    pos++;
                    c = buf[pos];
                }

                int numberLength = pos - numberStartPos;
                char *numberCStr = new char[numberLength + 1];
                strncpy(numberCStr, buf + numberStartPos, numberLength);
                numberCStr[numberLength] = 0;

                tokens.push_back(new ParserToken(numberCStr, PT_NUMBER, lineNumber));

                delete[] numberCStr;

            } else if(isSymbolCharacter(c)) {

                // Start of a symbol. Go to end of symbol.

                int symbolStartPos = pos;

                while(pos < length && isSymbolCharacter(c)) {
                    pos++;
                    c = buf[pos];
                }

                int symbolLength = pos - symbolStartPos;
                char *symbolCStr = new char[symbolLength + 1];
                strncpy(symbolCStr, buf + symbolStartPos, symbolLength);
                symbolCStr[symbolLength] = 0;

                tokens.push_back(new ParserToken(symbolCStr, PT_SYMBOL, lineNumber));

                delete[] symbolCStr;

            } else if(c == '"') {

                // Start of a quote block thinger.

                int consecutiveBackSlashes = 0;
                int quoteStartPos = pos;

                while(pos < length) {
                    pos++;
                    c = buf[pos];

                    if(c == '"') {

                        // As long as we have an even number of slashes,
                        // the quote isn't escaped. I think.
                        if(consecutiveBackSlashes % 2 == 0) {
                            break;
                        }
                    }

                    if(c == '\\') {
                        consecutiveBackSlashes++;
                    } else {
                        consecutiveBackSlashes = 0;
                    }
                }

                int quoteLength = pos - quoteStartPos;
                char *quoteCStr = new char[quoteLength + 1];
                strncpy(quoteCStr, buf + quoteStartPos, quoteLength);
                quoteCStr[quoteLength] = 0;

                tokens.push_back(new ParserToken(
                        stringUnescape<char>(quoteCStr + 1),
                        PT_STRING, lineNumber));

                delete[] quoteCStr;

                // Move OFF the last quote.
                pos++;

            } else {

                // All the single-character special tokens.

                ParserToken *pt = new ParserToken(c, lineNumber);

                switch(c) {

                case '{':
                    pt->type = PT_BLOCKSTART;
                    break;

                case '}':
                    pt->type = PT_BLOCKEND;
                    break;

                case '=':
                    pt->type = PT_EQUALS;
                    break;

                case ';':
                    pt->type = PT_LINEEND;
                    break;

                default:

                    // Spew out an error and bail out.
                    if(errorStr) {
                        ostringstream errStrStr;
                        errStrStr << "Unknown character token: \'" << c << "\' (0x" << std::hex << int(c) << ") on line " << lineNumber;
                        *errorStr = errStrStr.str();
                    }

                    delete pt;

                    // Clean up.
                    for(unsigned int i = 0; i < tokens.size(); i++) {
                        delete tokens[i];
                    }

                    return NULL;
                }

                tokens.push_back(pt);
                pos++;
            }

        }

        // At this point, concatenate strings that are adjacent to
        // each other. (Like in C, saying "Foo" "Bar" really means
        // "FooBar".)
        vector<ParserToken*> tokensCombined;
        unsigned int i = 0;
        while(i < tokens.size()) {

            while(i < tokens.size() && tokens[i]->type != PT_STRING) {
                tokensCombined.push_back(tokens[i]);
                tokens[i] = NULL;
                i++;
            }

            bool atLeastOne = false;
            int lineNo = -1;
            ostringstream str;
            while(i < tokens.size() && tokens[i]->type == PT_STRING) {
                if(!atLeastOne) {
                    atLeastOne = true;
                    lineNo = tokens[i]->lineNumber;
                }
                str << tokens[i]->str;
                i++;
            }

            if(atLeastOne) {
                ParserToken *pt = new ParserToken(str.str(), PT_STRING, lineNo);
                tokensCombined.push_back(pt);
            }
        }

        // Now do some simple not-really-grammars.

        pos = 0;

        ParserNode *node = parseTokens(tokensCombined, "root", &pos, errorStr);

        // Clean up. (Just string tokens left over from combining step.)
        for(unsigned int i = 0; i < tokens.size(); i++) {
            if(tokens[i]) {
                delete tokens[i];
            }
        }

        // Clean up.
        for(unsigned int i = 0; i < tokensCombined.size(); i++) {
            if(tokensCombined[i]) {
                delete tokensCombined[i];
            }
        }

        return node;

    }

    ParserNode *parseString(const std::string &str, std::string *errorStr) {

        return parseBuffer(str.c_str(), str.size(), errorStr);

    }

    // ParserNode *loadAndParse(const std::string &fileName, std::string *errorStr) {

    //     int length = 0;
    //     char *inBuf = FileSystem::loadFile(fileName, &length);
    //     ParserNode *node = NULL;

    //     if(inBuf) {
    //         node = parseBuffer(inBuf, length, errorStr);
    //         delete[] inBuf;
    //     } else if(errorStr) {
    //         *errorStr = "Failed to open file";
    //     }

    //     return node;
    // }

    ParserNode *ParserNode::clone() const
    {
        ParserNode *newNode = new ParserNode(getName());
        newNode->values = values;
        for(size_t i = 0; i < children.size(); i++) {
            ParserNode *newChild = nullptr;
            if(children[i]) {
                newChild = children[i]->clone();
            }
            newNode->children.push_back(newChild);
        }

        return newNode;
    }

}

