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

// This is a hierarchical text data format.

// It's hierarchical like XML, but without the horrible syntax. Having
// said that, there's a module for it to read in or write out XML-like
// syntax. It does not pretend to be standards compliant.

// There's also a JSON-like module, but the representation of the data
// gets a little weird. We don't have arrays the same way JSON does,
// so it fakes it with some specially named nodes, for instance.

// TODO: Some sort of long-path specification might help here, so we
// can access deeply nested fields a little easier.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <map>
#include <vector>
#include <string>
#include <sstream>

#include "malstring.h"
#include "filesystem.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Parser node. Stores information in a hierarchy and can be
    /// saved to or read from text.
    class ParserNode
    {
    public:

        /// Get the name of this node. Top-level nodes will be named
        /// "root" by default.
        std::string getName(void) const;

        /// Get the value for this as a boolean. Returns true if the
        /// value is defined, otherwise returns false and does not set
        /// the value.
        bool getBooleanValue(const std::string &name, bool *value) const;

        /// Get the value for this as a string. Returns true if the
        /// value is defined, otherwise returns false and does not set
        /// the value.
        bool getStringValue(const std::string &name, std::string *value) const;

        /// Get the value for this as a float. Returns true if the
        /// value is defined, otherwise returns false and does not set
        /// the value.
        bool getFloatValue(const std::string &name, float *value) const;

        /// Get the value for this as an integer. Returns true if the
        /// value is defined, otherwise returns false and does not set
        /// the value.
        bool getIntValue(const std::string &name, int *value) const;

        /// Decode a binary buffer from the string, treating it as a
        /// string of hex values for bytes. Calling function owns the
        /// buffer and is responsible for cleaning it up.
        bool getBinaryValue(const std::string &name, char **buf, int *length) const;

        /// Get a string value, but we don't care if it fails (and
        /// just returns an empty string.
        std::string getStringValueDirty(const std::string &name) const;

        /// Encode a buffer as hex values and store it as a value on
        /// the node.
        void setBinaryValue(const std::string &name, const char *buf, int length);

        // template<class T>
        // void setCustomValue(const std::string &name, const T* value);

        // template<class T>
        // bool getCustomValue(const std::string &name, T* value);

        /// Set a value that can be written to a normal ostream and
        /// read from an istream.
        template<class T>
        inline void setCustomValue(const std::string &valueName, const T* value) {
            std::ostringstream str;
            str << *value;
            values[valueName] = str.str();
        }

        /// Get a value in the same way as setCustomValue().
        template<class T>
        inline bool getCustomValue(const std::string &valueName, T* value) const {
            std::map<std::string, std::string>::const_iterator i =
                values.find(valueName);
            if(i != values.end()) {
                std::istringstream str((*i).second);
                str >> *value;
                return true;
            }
            return false;
        }

        /// Set the name of this node.
        void setName(const std::string &name);

        /// Set a value to "true" or "false".
        void setBooleanValue(const std::string &name, bool value);

        /// Set a value directly to a string.
        void setStringValue(const std::string &name, const std::string &value);

        /// Set a value to a string representation of a float.
        void setFloatValue(const std::string &name, float value);

        /// Set a value to a string representation of an integer.
        void setIntValue(const std::string &name, int value);

        /// Erase a value completely.
        void clearValue(const std::string &name);

        /// Get a list of all the values.
        void getValueNames(std::vector<std::string> &names) const;

        /// Get a child by its index.
        ParserNode *getChild(int index);
        const ParserNode *getChild(int index) const;

        /// Get the number of child nodes.
        int getNumChildren(void) const;

        /// Add a child to the end of the list of children.
        void addChildToEnd(ParserNode *node);

        /// Add a child to the start of the list of children (warning:
        /// slow).
        void addChildToStart(ParserNode *node);

        /// Output this ParserNode and all its children to an output
        /// stream in a format readable by parseBuffer() and
        /// parseString().
        void output(std::ostream &out, int indentLevel) const;

        /// Output this ParserNode and all its children in JSON. NOTE:
        /// Because of the nature of JSON involving key/value pairs
        /// instead of arbitrarily named children, all similarly-named
        /// children will be grouped into JSON arrays. Order relative
        /// to other elements may be lost. Also, make sure you don't
        /// have children and attributes with the same name.
        void outputJson(
            std::ostream &out,
            int indentLevel,
            bool lineStart = true) const;

        /// Output this ParserNode and all its children to an XML-like
        /// format.
        void outputXml(std::ostream &out, int indentLevel) const;

        /// Destructor.
        ~ParserNode(void);

        /// Default constructor.
        ParserNode(void);

        /// Constructor with a name.
        ParserNode(const std::string &name);

        /// Output this ParserNode as a string. Readable by
        /// parseString().
        std::string toString(void) const;

        /// Get the index of a child, optionally after some other index.
        int getChildIndexByName(const std::string &name, int after = -1);

        /// Just get the first child with this name.
        ParserNode *getChildByName(const std::string &name);

        /// Clone the entire tree.
        ParserNode *clone() const;

    private:

        std::map<std::string, std::string> values;
        std::vector<ParserNode*> children;
        std::string name;

    };

    /// Parse a buffer and get a ParserNode tree. Returns NULL on
    /// error and sets an error description in errorStr if errorStr is
    /// not NULL.
    ParserNode *parseBuffer(const char *buf, int length, std::string *errorStr = NULL);

    /// Parse a generic std::string and get a ParserNode tree. Returns
    /// NULL on error and sets an error description in errorStr if
    /// errorStr is not NULL.
    ParserNode *parseString(const std::string &str, std::string *errorStr = NULL);

    /// Parse a JSON string. Not standards compliant.
    ParserNode *parseJsonString(const std::string &str, std::string *errorStr = NULL);

    /// Output to an ostream.
    std::ostream &operator<<(std::ostream &out, const ParserNode &node);

    // TODO: istream input.

    /// Load and parse a file.
    ParserNode *loadAndParse(const std::string &fileName, std::string *errorStr = NULL);

    /// Internal token. Used by multiple parser modules.
    class ParserToken
    {
    public:
        ParserToken(const std::string &str, int type, int lineNumber);
        ParserToken(char c, int lineNumber);
        std::string str;
        int type;
        int lineNumber;
    private:
    };

}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    // Utility functions.

    inline bool parserIsSymbolCharacter(char c)
    {
        if((c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_')) {

            return true;
        }

        return false;
    }

    inline bool parserIsNumberCharacter(char c)
    {
        if((c >= '0' && c <= '9') ||
           c == '.' ||
           c == '-') {

            return true;
        }

        return false;
    }

    enum ParserTokenType
    {
        PT_UNKNOWN,

        PT_BLOCKSTART,
        PT_BLOCKEND,

        PT_EQUALS,
        PT_LINEEND,

        PT_SYMBOL,
        PT_STRING,
        PT_NUMBER,
    };

    inline void parserIndentOutput(std::ostream &out, int indentLevel)
    {
        for(int i = 0; i < indentLevel; i++) {
            out << "    ";
        }
    }

    inline std::string escapeCDATA(const std::string &str)
    {
        std::ostringstream outStr;

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

    // ParserNode implementation.

    inline std::string ParserNode::getName(void) const
    {
        return name;
    }

    inline void ParserNode::setName(const std::string &name)
    {
        this->name = name;
    }

    inline ParserNode *ParserNode::getChild(int index)
    {
        if(index >= (int)children.size() || index < 0) {
            return NULL;
        }
        return children[index];
    }

    inline const ParserNode *ParserNode::getChild(int index) const
    {
        if(index >= (int)children.size() || index < 0) {
            return NULL;
        }
        return children[index];
    }

    inline int ParserNode::getNumChildren(void) const
    {
        return (int)children.size();
    }

    inline void ParserNode::addChildToEnd(ParserNode *node)
    {
        children.push_back(node);
    }

    inline void ParserNode::addChildToStart(ParserNode *node)
    {
        children.insert(children.begin(), node);
    }

    inline void ParserNode::output(std::ostream &out, int indentLevel) const
    {
        // Print all values.
        bool atLeastOneValue = false;
        for(std::map<std::string, std::string>::const_iterator i = values.begin(); i != values.end(); i++) {
            parserIndentOutput(out, indentLevel);

            std::string escapedOut = stringEscape((*i).second);

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
                out << (*i).first << " = " << std::endl;

                std::string indented;
                for(int j = 0; j < indentLevel + 1; j++) {
                    indented = indented + "    ";
                }

                escapedOut = stringEscape(
                    // (*i).second, "\\n\"\n" + indented + "\"");
                    (*i).second, true);

                out << indented << "\"" << escapedOut << "\";" << std::endl;

            } else {

                // We can fit this on one line.
                out << (*i).first << " = \"" << escapedOut << "\";" << std::endl;
            }

            atLeastOneValue = true;
        }

        // Add some space before the children.
        if(atLeastOneValue && children.size()) {
            out << std::endl;
        }

        // Print all children.
        for(size_t cn = 0; cn < children.size(); cn++) {

            parserIndentOutput(out, indentLevel);

            if(children[cn]->name.size()) {
                out << children[cn]->name << " ";
            } else {
                out << "null " << std::endl;
            }

            out << "{" << std::endl;

            children[cn]->output(out, indentLevel + 1);

            parserIndentOutput(out, indentLevel);
            out << "}" << std::endl;

            // Add a line between children.
            if(cn + 1 != children.size()) {
                out << std::endl;
            }
        }
    }

    inline void ParserNode::outputXml(std::ostream &out, int indentLevel) const
    {
        parserIndentOutput(out, indentLevel);

        bool headerOnly = false;
        if(getName() == "_root") {
            headerOnly = true;
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;

            // This will actually wrap around to 0xFFFFFFFF or
            // somesuch, but whatever.
            indentLevel--;
        }

        if(getName() == "_text") {

            std::string outStr;

            // Check for leading or trailing whitespace so we know if
            // we need CDATA.
            bool useCDATA = false;
            std::string textStr = getStringValueDirty("text");
            if(textStr.size() && (isWhiteSpace(textStr[0]) || isWhiteSpace(textStr[textStr.size() - 1]))) {
                useCDATA = true;
            }

            if(useCDATA) {
                out << "<![CDATA[" << escapeCDATA(textStr) << "]]>" << std::endl;
            } else {
                out << stringXmlEscape(textStr) << std::endl;
            }

        } else {

            bool hadAttributes = false;

            if(!headerOnly) {

                out << "<" << getName();

                // Output attributes.
                bool firstAttrib = false;
                for(std::map<std::string, std::string>::const_iterator i = values.begin(); i != values.end(); i++) {

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
                    out << ">" << std::endl;
                }

                for(int i = 0; i < getNumChildren(); i++) {
                    children[i]->outputXml(out, indentLevel + 1);
                }

                parserIndentOutput(out, indentLevel);

                // End tag.
                if(!headerOnly) {
                    out << "</" << getName() << ">" << std::endl;
                }

            } else {

                if(!headerOnly) {
                    out << (hadAttributes ? " " : "") << "/>" << std::endl;
                }
            }
        }
    }

    inline ParserNode::~ParserNode(void)
    {
        for(unsigned int cn = 0; cn < children.size(); cn++) {
            delete children[cn];
        }
    }

    inline std::string ParserNode::toString(void) const
    {
        std::ostringstream str;
        output(str, 0);
        return str.str();
    }

    inline bool ParserNode::getBooleanValue(const std::string &name, bool *value) const
    {
        if(values.count(name)) {
            std::string val = values.find(name)->second;
            if(val == "false" || val == "0" || val == "") {
                *value = false;
            } else {
                *value = true;
            }
            return true;
        }
        return false;
    }

    inline bool ParserNode::getStringValue(const std::string &name, std::string *value) const
    {
        if(values.count(name)) {
            std::string val = values.find(name)->second;
            *value = val;
            return true;
        }
        return false;
    }

    inline std::string ParserNode::getStringValueDirty(const std::string &name) const
    {
        std::string str = "";
        getStringValue(name, &str);
        return str;
    }

    inline bool ParserNode::getFloatValue(const std::string &name, float *value) const
    {
        if(values.count(name)) {
            std::string val = values.find(name)->second;
            *value = atof(val.c_str());
            return true;
        }
        return false;
    }

    inline bool ParserNode::getIntValue(const std::string &name, int *value) const
    {
        if(values.count(name)) {
            std::string val = values.find(name)->second;
            *value = atoi(val.c_str());
            return true;
        }
        return false;
    }

    inline bool ParserNode::getBinaryValue(const std::string &name, char **buf, int *length) const
    {
        if(values.count(name)) {
            *buf = strDecodeHex(values.find(name)->second, length);
            return true;
        }
        return false;
    }

    inline void ParserNode::setBinaryValue(const std::string &name, const char *buf, int length)
    {
        strEncodeHex(buf, length, values[name], 40);
    }

    inline void ParserNode::setBooleanValue(const std::string &name, bool value)
    {
        std::ostringstream str;
        str << (value ? "true" : "false");
        values[name] = str.str();
    }

    inline void ParserNode::setStringValue(const std::string &name, const std::string &value)
    {
        values[name] = value;
    }

    inline void ParserNode::setFloatValue(const std::string &name, float value)
    {
        std::ostringstream str;
        str << value;
        values[name] = str.str();
    }

    inline void ParserNode::setIntValue(const std::string &name, int value)
    {
        std::ostringstream str;
        str << value;
        values[name] = str.str();
    }

    inline void ParserNode::clearValue(const std::string &name)
    {
        values.erase(name);
    }

    inline void ParserNode::getValueNames(std::vector<std::string> &names) const
    {
        for(std::map<std::string, std::string>::const_iterator i = values.begin();
            i != values.end(); i++) {
            names.push_back((*i).first);
        }
    }

    inline int ParserNode::getChildIndexByName(const std::string &name, int after)
    {
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

    inline ParserNode *ParserNode::getChildByName(const std::string &name)
    {
        return getChild(getChildIndexByName(name));
    }

    inline std::ostream &operator<<(std::ostream &out, const ParserNode &node)
    {
        node.output(out, 0);
        return out;
    }

    inline ParserNode::ParserNode(void)
    {
        name = "null";
    }

    inline ParserNode::ParserNode(const std::string &name)
    {
        this->name = name;
    }

    inline ParserNode *ParserNode::clone() const
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

    // Parser implementation.

    inline ParserToken::ParserToken(const std::string &str, int type, int lineNumber)
    {
        this->str = str;
        this->type = type;
        this->lineNumber = lineNumber;
    }

    inline ParserToken::ParserToken(char c, int lineNumber)
    {
        // This seems oddly the simplest way to do this...
        str = " ";
        str[0] = c;
        this->type = PT_UNKNOWN;
        this->lineNumber = lineNumber;
    }

    inline ParserNode *parseTokens(
        const std::vector<ParserToken*> &tokens,
        const std::string &nodeName,
        int *pos, std::string *errorStr)
    {
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

                std::string childNodeName = tokens[*pos]->str;

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
                    std::ostringstream errStrStr;
                    errStrStr << "Syntax error at: \"" << tokens[*pos]->str << "\" on line " << tokens[*pos]->lineNumber;
                    *errorStr = errStrStr.str();
                }

                delete node;
                return NULL;

            }

        }

        return node;
    }

    inline ParserNode *parseBuffer(const char *buf, int length, std::string *errorStr)
    {
        // First, tokenize.

        std::vector<ParserToken*> tokens;

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

            } else if(parserIsNumberCharacter(c)) {

                // Number. Go to end of number.

                int numberStartPos = pos;

                while(pos < length && parserIsNumberCharacter(c)) {
                    pos++;
                    c = buf[pos];
                }

                int numberLength = pos - numberStartPos;
                char *numberCStr = new char[numberLength + 1];
                strncpy(numberCStr, buf + numberStartPos, numberLength);
                numberCStr[numberLength] = 0;

                tokens.push_back(new ParserToken(numberCStr, PT_NUMBER, lineNumber));

                delete[] numberCStr;

            } else if(parserIsSymbolCharacter(c)) {

                // Start of a symbol. Go to end of symbol.

                int symbolStartPos = pos;

                while(pos < length && parserIsSymbolCharacter(c)) {
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
                        std::ostringstream errStrStr;
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
        std::vector<ParserToken*> tokensCombined;
        unsigned int i = 0;
        while(i < tokens.size()) {

            while(i < tokens.size() && tokens[i]->type != PT_STRING) {
                tokensCombined.push_back(tokens[i]);
                tokens[i] = NULL;
                i++;
            }

            bool atLeastOne = false;
            int lineNo = -1;
            std::ostringstream str;
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

    inline ParserNode *parseString(const std::string &str, std::string *errorStr)
    {
        return parseBuffer(str.c_str(), str.size(), errorStr);
    }

    inline ParserNode *loadAndParse(const std::string &fileName, std::string *errorStr)
    {
        ParserNode *node = nullptr;
        std::string fileData = FileSystem::loadFileString(fileName);

        if(fileData.size()) {
            node = parseString(fileData, errorStr);
        } else if(errorStr) {
            *errorStr = "Failed to open file";
        }

        return node;
    }
}

