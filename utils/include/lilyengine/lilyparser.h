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

#pragma once

#include <map>
#include <vector>
#include <string>
#include <sstream>

namespace ExPop {

    /// Parser node. Stores information in a hierarchy and can be
    /// saved to or read from text.
    class ParserNode {

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

        /// Output this ParserNode and all its children to an XML
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

    /// Parse a string of XML stuff. Not fully standards
    /// compliant. Probably not even mostly standards
    /// compliant. Actually, it's more just an XML-like language.
    ParserNode *parseXmlString(const std::string &str, std::string *errorStr = NULL);

    /// Parse a JSON string. Not fully standards compliant.
    ParserNode *parseJsonString(const std::string &str, std::string *errorStr = NULL);

    /// Output to an ostream.
    std::ostream &operator<<(std::ostream &out, const ParserNode &node);

    // TODO: istream input.

    // /// Load and parse a file.
    // ParserNode *loadAndParse(const std::string &fileName, std::string *errorStr = NULL);

    /// Internal token. Used by multiple parser modules.
    class ParserToken {
    public:
        ParserToken(const std::string &str, int type, int lineNumber);
        ParserToken(char c, int lineNumber);
        std::string str;
        int type;
        int lineNumber;
    private:
    };

}
