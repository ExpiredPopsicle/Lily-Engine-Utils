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

// GraphicalConsole internal implementation.

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#pragma once

namespace ExPop
{

    // This function will parse a single argument from a string.
    // Parameters are space-delimeted unless they are inside a quoted
    // string. If they are in a quoted string, then backslash escape
    // characters will be in effect.
    inline std::string graphicalConsoleReadParam(std::string &str)
    {
        // Skip any initial whitespace.
        size_t start = 0;
        while(start < str.size() && ExPop::isWhiteSpace(str[start])) {
            start++;
        }

        size_t end = start;
        std::string ret;

        // Check for quoted string.
        if(start < str.size() && str[start] == '"') {

            // Quoted string. Search for an unescaped quote.
            end++;
            bool escaped = false;
            while(end < str.size()) {

                if(str[end] == '\\') {
                    escaped = !escaped;
                } else {
                    if(!escaped && str[end] == '"') {
                        break;
                    } else {
                        escaped = false;
                    }
                }

                end++;
            }

            ret = str.substr(start, end - start);

            // Skip past the final '"' if we're on one.
            if(end < str.size() && str[end] == '"') {
                end++;
            }

            // Strip off start and end quotes.
            if(ret.size() && ret[ret.size() - 1] == '"') {
                ret.erase(ret.end() - 1);
            }
            if(ret.size() && ret[0] == '"') {
                ret.erase(ret.begin());
            }

            // Un-escape.
            ret = ExPop::stringUnescape(ret);

        } else {

            // Skip until we find whitespace or the end.
            while(end < str.size() && !ExPop::isWhiteSpace(str[end])) {
                end++;
            }

            // See if we ended up on a space.
            bool onSpace = false;
            if(end < str.size() && ExPop::isWhiteSpace(str[end])) {
                onSpace = true;
            }

            ret = str.substr(start, end - start);
        }

        // Chop off what we just consumed.
        str = str.substr(end);

        return ret;
    }

    inline std::vector<std::string> graphicalConsoleSplitLine(const std::string &line)
    {
        std::string utf8Line = line;
        std::vector<std::string> ret;

        // Split the line of text up into parameters. This is
        // more complicated than normal tokenizing because of
        // the quoted string support.
        while(utf8Line.size()) {
            std::string paramStr = graphicalConsoleReadParam(utf8Line);
            if(paramStr.size()) {
                ret.push_back(paramStr);
            }
        }

        return ret;
    }

    template<typename T>
    inline T graphicalConsoleDecodeParameter(const std::string &str)
    {
        T ret;
        std::istringstream istr(str);
        istr >> ret;
        return ret;
    }

    template<>
    inline std::string graphicalConsoleDecodeParameter<std::string>(const std::string &str)
    {
        return str;
    }

    inline std::string graphicalConsoleGetRedErrorText()
    {
        return "\e[31;1merror\e[0m";
    }

}
