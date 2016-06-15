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

// XML-like parser for the Lily-Engine-Utils hierarchical data format
// module.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

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

}



