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

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>

namespace ExPop {

    // TODO: Make a consistent prefix for this stuff (str vs
    // string). Possibly toss it into a namespace under ExPop.

    /// Return true if the character is whitespace. False otherwise.
    template<typename T>
    inline bool isWhiteSpace(T c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    /// Simple function to see if a string starts with another string.
    /// (needle is the string we're looking for, haystack is the
    /// string we're looking in).
    template<typename T>
    inline bool stringStartsWith(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack)
    {
        if(haystack.size() < needle.size()) return false;
        for(size_t i = 0; i < needle.size(); i++) {
            if(needle[i] != haystack[i]) {
                return false;
            }
        }
        return true;
    }

    /// Return true if one string ends with another string. (needle is
    /// the string we're looking for, haystack is the string we're
    /// looking in).
    template<typename T>
    inline bool stringEndsWith(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack)
    {
        if(haystack.size() < needle.size()) return false;
        for(size_t i = 0; i < needle.size(); i++) {
            if(needle[i] != haystack[haystack.size() - needle.size() + i]) {
                return false;
            }
        }
        return true;
    }

    /// Generate tokens from a string. Stores saved tokens into the
    /// passed-in tokens parameter.
    void stringTokenize(
        const std::string &str,
        const std::string &delims,
        std::vector<std::string> &tokens,
        bool allowEmpty = false);

    /// Escape a string. newLineReplace will be used to replace new
    /// lines.
    template<typename T>
    inline std::basic_string<T> stringEscape(
        const std::basic_string<T> &str,
        bool replaceNewlines = true)
    {
        std::basic_ostringstream<T> outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            switch(str[i]) {

                case '"':
                    outStr << '\\' << '\"';
                    break;

                case '\\':
                    outStr << '\\' << '\\';
                    break;

                case '\n':
                    if(replaceNewlines) {
                        outStr << '\\' << 'n';
                    } else {
                        outStr << str[i];
                    }
                    break;

                case '\r':
                    outStr << '\\' << 'r';
                    break;

                default:
                    outStr << str[i];
                    break;
            }

        }

        return outStr.str();
    }

    /// Unescape a string.
    template<typename T>
    inline std::basic_string<T> stringUnescape(
        const std::basic_string<T> &str)
    {
        std::basic_ostringstream<T> outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            if(str[i] == '\\') {

                i++;

                if(i >= str.size()) break;

                switch(str[i]) {

                    case '\\':
                        outStr << '\\';
                        break;

                    case '"':
                        outStr << '\"';
                        break;

                    case 'n':
                        outStr << '\n';
                        break;

                    case 'r':
                        outStr << '\r';
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

    /// Escape a string by adding some XML entities. Only a few of
    /// them.
    std::string stringXmlEscape(const std::string &str);

    /// Not-really-compliant XML string unescape. Handles XML/HTML
    /// special entities. But only a few of them, and ASCII-range
    /// numerical special values.
    std::string stringXmlUnescape(const std::string &str);

    /// Encode a binary buffer as a string of hex values..
    void strEncodeHex(const void *buf, int length, std::string &str, int columns = 80);

    /// Decode a string of hex values. Ownership of the buffer given
    /// to the caller.
    char *strDecodeHex(const std::string &str, int *length);

    /// Split a string at the first occurence of some dividing
    /// substring.
    void stringSplit(
        const std::string &str,
        const std::string &divider,
        std::string &out1,
        std::string &out2,
        bool startFromEnd = false);

    /// Parse a URI in a probably not-standard way. Returns true on
    /// success.
    bool parseUri(
        const std::string &input,
        std::string &outScheme,
        std::string &outAuthority,
        std::string &outPath,
        std::string &outQuery,
        std::string &outFragment);

    // TODO: Switch all UTF-32 vectors from unsigned int type over to
    // uint32_t.

    /// Convert a UTF-8 string to a UTF-32 string.
    std::basic_string<uint32_t> strUTF8ToUTF32(
        const std::string &utf8Str);

    /// Convert a UTF-32 string to a UTF-8 string.
    std::string strUTF32ToUTF8(
        const std::vector<unsigned int> &utf32Str);

    /// Return a string with all the trailing and leading whitespace
    /// removed.
    template<typename T>
    std::basic_string<T> stringTrim(const std::basic_string<T> &str)
    {
        if(!str.size()) {
            return std::basic_string<T>();
        }

        // Find the start (first non-whitespace character).
        unsigned int start = 0;
        while(start < str.size() && isWhiteSpace(str[start])) {
            start++;
        }

        // All whitespace? Return now. Otherwise we'll end up with
        // some serious issues when the end is the start and the start
        // is the end.
        if(start == str.size()) {
            return std::basic_string<T>();
        }

        // Find the end.
        size_t end = str.size() - 1;
        while(isWhiteSpace(str[end]) && end != 0) {
            end--;
        }

        // Just return the substring.
        return str.substr(start, end - start + 1);
    }

    /// Returns a word-wrapped version of the input string, wrapped at
    /// the number of characters in columns.
    std::string strWordWrap(
        const std::string &str,
        unsigned int columns,
        unsigned int columnsAfterFirstLine = 0);

    /// Indent a block of text.
    std::string strIndent(
        const std::string &str,
        unsigned int firstRow,
        unsigned int afterFirstRow);

    /// Returns a string with each line appended at the beginning with
    /// some prefix string.
    std::string strPrefixLines(
        const std::string &str,
        const std::string &prefix);

    // Base64 stuff.

    /// Decode a buffer from a base64 string as specified in RFC 2045.
    /// Returns a pointer to a buffer allocated with new.
    /// Responsibility for freeing this buffer with delete[] is up to
    /// the caller.
    unsigned char *strBase64Decode(const std::string &str, unsigned int *length);

    /// Decode a string from a base64 string. Convenience function for
    /// text data.
    std::string strBase64DecodeString(const std::string &str);

    /// Encode a buffer to a base64 string.
    std::string strBase64Encode(const void *buffer, size_t length);

    /// Encode a string to a base64 string. Convenience function for
    /// text data.
    std::string strBase64EncodeString(const std::string &str);

    template<typename T>
    inline bool stringCompareOffset(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack,
        size_t haystackOffset)
    {
        // String length 0 matches anything, including a pointer to
        // just after the last character.
        if(needle.size() == 0 && haystackOffset <= haystack.size()) {
            return true;
        }

        // Anything in needle won't match a pointer to after the last
        // character, or further.
        if(haystackOffset >= haystack.size()) {
            return false;
        }

        // Offset + needle would put needle hanging off the end of the
        // haystack. Obvious failure.
        if(haystackOffset + needle.size() >= haystack.size()) {
            return false;
        }

        // We proooooobably don't have to do this check, unless
        // haystack really does take up almost the entire address
        // space some how.
      #if 0
        if(haystackOffset > ~(size_t)0 - needle.size()) {
            return false;
        }
      #endif

        // Now do the actual comparison.
        for(size_t i = 0; i < needle.size(); i++) {
            if(haystack[i + haystackOffset] != needle[i]) {
                return false;
            }
        }

        return true;
    }

    template<typename T>
    inline std::basic_string<T> stringReplace(
        const std::basic_string<T> &stringToReplace,
        const std::basic_string<T> &replacement,
        const std::basic_string<T> &sourceText)
    {
        assert(stringToReplace.size());

        std::basic_string<T> output;
        for(size_t i = 0; i < sourceText.size(); i++) {

            if(stringCompareOffset(stringToReplace, sourceText, i)) {

                // Found the string to replace.
                output = output + replacement;
                i += stringToReplace.size() - 1;

            } else {

                // No replacement. Just append characters.
                output = output + sourceText.substr(i, 1);

            }

        }

        return output;
    }



    /// Simple buffer that we can dynamically expand or iterate through.
    class SimpleBuffer {
	public:

		SimpleBuffer(void);
		SimpleBuffer(const void *data, size_t length, bool myOwnData = true);
		~SimpleBuffer(void);

        /// Add some data onto the end. Reallocates if needed. Note:
        /// Doesn't just double the size of the buffer. It only
        /// allocates as much as it needs so continuously adding to
        /// the buffer in this way can be slow.
		void addData(const void *buffer, size_t length);

        /// Just get a pointer to the start of the data.
		const char *getData(void) const;

        /// Get the current length of the data.
		size_t getLength(void) const;

        /// Reads a number of bytes equal to the bytes parameter into
        /// the memory pointed to by dst and advances the read pointer
        /// by that amount. Returns a pointer to the position the read
        /// pointer ends at.
		const char *getDataAndAdvance(void *dst, size_t bytes);

        /// Moves the read pointer back to the start of the data.
        void seekToStart(void);

        int compare(const void *buffer, size_t length);

        void clear(void);

	private:

        // If this is true, the data will be cleaned up when this goes
        // away.
        bool myOwnData;

		char *data;
		size_t length;
		size_t readPtr;
    };



    inline void doStringTests()
    {
        assert(stringStartsWith<char>("dick", "dickbutts"));
        assert(stringEndsWith<char>("butts", "dickbutts"));
        assert(!stringEndsWith<char>("dick", "dickbutts"));
        assert(!stringStartsWith<char>("butts", "dickbutts"));
        assert(stringUnescape<char>("foo\\n\\\\") == "foo\n\\");
        assert("foo\\n\\\\" == stringEscape<char>("foo\n\\"));
        assert(stringTrim<char>("  asdf") == std::string("asdf"));
        assert(stringTrim<char>("asdf  ") == std::string("asdf"));
        assert(stringTrim<char>("  asdf  ") == std::string("asdf"));
        assert(stringTrim<char>("asdf") == std::string("asdf"));
        assert(stringTrim<char>("  ") == std::string(""));
        assert(stringTrim<char>("") == std::string(""));
    }
};

