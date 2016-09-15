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

// This is a string library that operates on std::string and other
// variants of std::basic_string.

// std::basic_string<uint32_t> is used as a UTF-32 type here, because
// UTF-32 is actually fixed-bytes-per-character, unline UTF-16.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <cstring>
#include <cstdlib>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    // TODO: Make a consistent prefix for this stuff (str vs
    // string). Possibly toss it into a namespace under ExPop.

    /// Return true if the character is whitespace. False otherwise.
    template<typename T>
    inline bool isWhiteSpace(T c);

    /// Simple function to see if a string starts with another string.
    /// (needle is the string we're looking for, haystack is the
    /// string we're looking in).
    template<typename T>
    inline bool stringStartsWith(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack);

    /// Return true if one string ends with another string. (needle is
    /// the string we're looking for, haystack is the string we're
    /// looking in).
    template<typename T>
    inline bool stringEndsWith(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack);

    /// Generate tokens from a string. Stores saved tokens into the
    /// passed-in tokens parameter.
    inline void stringTokenize(
        const std::string &str,
        const std::string &delims,
        std::vector<std::string> &tokens,
        bool allowEmpty = false);

    /// Escape a string. newLineReplace will be used to replace new
    /// lines.
    template<typename T>
    inline std::basic_string<T> stringEscape(
        const std::basic_string<T> &str,
        bool replaceNewlines = true);

    /// Unescape a string.
    template<typename T>
    inline std::basic_string<T> stringUnescape(
        const std::basic_string<T> &str);

    /// Escape a string by adding some XML entities. Only a few of
    /// them.
    inline std::string stringXmlEscape(const std::string &str);

    /// Not-really-compliant XML string unescape. Handles XML/HTML
    /// special entities. But only a few of them, and ASCII-range
    /// numerical special values.
    inline std::string stringXmlUnescape(const std::string &str);

    /// Encode a binary buffer as a string of hex values..
    inline void strEncodeHex(const void *buf, int length, std::string &str, int columns = 80);

    /// Decode a string of hex values. Ownership of the buffer given
    /// to the caller.
    inline char *strDecodeHex(const std::string &str, int *length);

    /// Split a string at the first occurence of some dividing
    /// substring.
    inline void stringSplit(
        const std::string &str,
        const std::string &divider,
        std::string &out1,
        std::string &out2,
        bool startFromEnd = false);

    /// Parse a URI in a probably not-standard way. Returns true on
    /// success.
    inline bool stringParseUri(
        const std::string &input,
        std::string &outScheme,
        std::string &outAuthority,
        std::string &outPath,
        std::string &outQuery,
        std::string &outFragment);

    // TODO: Switch all UTF-32 vectors from unsigned int type over to
    // uint32_t.

    /// Convert a UTF-8 string to a UTF-32 string.
    inline std::basic_string<uint32_t> stringUTF8ToUTF32(
        const std::string &utf8Str);

    /// Convert a UTF-32 string to a UTF-8 string.
    inline std::string stringUTF32ToUTF8(
        const std::vector<uint32_t> &utf32Str);

    /// Return a string with all the trailing and leading whitespace
    /// removed.
    template<typename T>
    inline std::basic_string<T> stringTrim(const std::basic_string<T> &str);

    /// Returns a word-wrapped version of the input string, wrapped at
    /// the number of characters in columns.
    inline std::string stringWordWrap(
        const std::string &str,
        unsigned int columns,
        unsigned int columnsAfterFirstLine = 0);

    /// Indent a block of text.
    inline std::string stringIndent(
        const std::string &str,
        unsigned int firstRow,
        unsigned int afterFirstRow);

    /// Returns a string with each line appended at the beginning with
    /// some prefix string.
    inline std::string stringPrefixLines(
        const std::string &str,
        const std::string &prefix);

    /// Compare a string to a substring inside another one. Returns
    /// true if the strings match. Case sensitive. Unlike using
    /// std::string::substr() and comparing the result, this will just
    /// return false if you specify a bad range.
    template<typename T>
    inline bool stringCompareOffset(
        const std::basic_string<T> &needle,
        const std::basic_string<T> &haystack,
        size_t haystackOffset);

    /// Do a simple string replacement. Returns the modified string.
    template<typename T>
    inline std::basic_string<T> stringReplace(
        const std::basic_string<T> &stringToReplace,
        const std::basic_string<T> &replacement,
        const std::basic_string<T> &sourceText);

    /// tolower(), now on full strings.
    template<typename T>
    inline std::basic_string<T> stringToLower(
        const std::basic_string<T> &str);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    template<typename T>
    inline bool isWhiteSpace(T c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

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

    template<typename T>
    inline std::basic_string<T> stringEscape(
        const std::basic_string<T> &str,
        bool replaceNewlines)
    {
        std::basic_ostringstream<T> outStr;

        for(size_t i = 0; i < str.size(); i++) {

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

    template<typename T>
    inline std::basic_string<T> stringUnescape(
        const std::basic_string<T> &str)
    {
        std::basic_ostringstream<T> outStr;

        for(size_t i = 0; i < str.size(); i++) {

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

    template<typename T>
    inline std::basic_string<T> stringTrim(const std::basic_string<T> &str)
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
        if(haystackOffset + needle.size() > haystack.size()) {
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

        // Early-out for obvious situations.
        if(stringToReplace.size() > sourceText.size()) {
            return sourceText;
        }

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

    inline void stringTokenize(
        const std::string &str, const std::string &delims,
        std::vector<std::string> &tokens, bool allowEmpty)
    {
        size_t i = 0;
        while(i < str.size()) {

            bool firstDelim = true;

            // Skip over delimeters.
            while(i < str.size() && strchr(delims.c_str(), str[i])) {
                i++;

                if(!firstDelim && allowEmpty) {
                    tokens.push_back("");
                }

                firstDelim = false;
            }

            // Mark the start of a token.
            int tokStart = i;

            // Skip to the next delimeter.
            while(i < str.size() && !strchr(delims.c_str(), str[i])) {
                i++;
            }

            // Mark the end.
            int tokEnd = i;

            // Copy the token.
            if(tokStart != tokEnd) {
                std::string token = str.substr(tokStart, (tokEnd - tokStart));
                tokens.push_back(token);
            }
        }
    }

    inline std::string stringXmlEscape(const std::string &str)
    {
        std::ostringstream outStr;

        for(size_t i = 0; i < str.size(); i++) {

            // TODO: Add a case for completely bizarre Unicode
            // stuff. (Inverse of #xNUMBER;)

            switch(str[i]) {

                case '"':
                    outStr << "&quot;";
                    break;

                case '<':
                    outStr << "&lt;";
                    break;

                case '>':
                    outStr << "&gt;";
                    break;

                case '&':
                    outStr << "&amp;";
                    break;

                default:
                    outStr << str[i];
                    break;
            }

        }

        return outStr.str();
    }

    inline std::string stringXmlUnescape(const std::string &str)
    {
        std::ostringstream outStr;

        for(size_t i = 0; i < str.size(); i++) {

            if(str[i] == '&') {

                i++;

                if(i >= str.size()) break;

                if(stringStartsWith<char>("amp;", str.c_str() + i)) {

                    i += 3;
                    outStr << '&';

                } else if(stringStartsWith<char>("quot;", str.c_str() + i)) {

                    i += 4;
                    outStr << '"';

                } else if(stringStartsWith<char>("lt;", str.c_str() + i)) {

                    i += 2;
                    outStr << '<';

                } else if(stringStartsWith<char>("gt;", str.c_str() + i)) {

                    i += 2;
                    outStr << '>';

                } else if(str[i] == '#') {

                    i++;

                    bool hexMode = false;

                    if(i >= str.size()) break;
                    if(str[i] == 'x') {
                        hexMode = true;
                        i++;
                    }

                    // Read in a hex value until we hit ';'
                    std::ostringstream hexStr;
                    while(i < str.size() && str[i] != ';' && (
                          (str[i] >= '0' && str[i] <= '9') ||
                          (str[i] >= 'a' && str[i] <= 'f') ||
                          (str[i] >= 'A' && str[i] <= 'F'))) {

                        hexStr << str[i];
                        i++;

                    }

                    uint32_t hexVal =
                        strtol(hexStr.str().c_str(), NULL, hexMode ? 16 : 10);

                    // TODO: Handle unicode and extended stuff better
                    // than just ignoring it if it's out of range!
                    if(hexVal <= 255) {
                        outStr << ((char)hexVal);
                    }
                }


            } else {

                outStr << str[i];

            }
        }

        return outStr.str();
    }

    inline int stringGetNextNonWhiteSpace(const std::string &str, int start) {

        if(start >= int(str.size())) return -1;

        while(isWhiteSpace(str[start])) {
            start++;
            if(start >= int(str.size())) return -1;
        }

        return start;
    }


    inline char stringNibbleToHex(uint32_t n)
    {
        if(n < 0xa) {
            return '0' + n;
        }
        return ('a' + n) - 0xa;
    }

    inline uint32_t stringHexToNibble(char h)
    {
        if(h >= '0' && h <= '9') {
            return h - '0';
        } else if(h >= 'A' && h <= 'F') {
            return h - 'A' + 0xa;
        } else if(h >= 'a' && h <= 'f') {
            return h - 'a' + 0xa;
        }

        // Derp?
        return 0;

    }

    inline void strEncodeHex(const void *buf, int length, std::string &str, int columns)
    {
        str.resize((length * 2) + ((length * 2) / columns), 0);

        int strPos = 0;
        int lineCounter = 0;
        const char *cbuf = (const char *)buf;

        for(int i = 0; i < length; i++) {
            str[strPos++] = stringNibbleToHex((cbuf[i] & 0xf0) >> 4);
            str[strPos++] = stringNibbleToHex((cbuf[i] & 0x0f));

            lineCounter += 2;

            if(lineCounter > columns) {
                str[strPos++] = '\n';
                lineCounter = 0;
            }
        }

        str.resize(strPos);

    }

    inline char *strDecodeHex(const std::string &str, int *length)
    {
        // Do this in two passes. First, count up the number of bytes.
        int pos = 0;

        int hexPos = 0;
        char hexDigits[2];
        int numBytes = 0;

        while(pos < int(str.size())) {
            pos = stringGetNextNonWhiteSpace(str, pos);

            if(pos == -1) {
                break;
            }

            hexDigits[hexPos] = str[pos];
            hexPos++;

            if(hexPos >= 2) {
                hexPos = 0;
                numBytes++;
            }

            pos++;
        }

        // Allocate the buffer.
        *length = numBytes;
        char *retBuf = new char[numBytes];

        hexPos = 0;
        pos = 0;
        int bufPos = 0;

        // Do it again.
        while(pos < int(str.size())) {
            pos = stringGetNextNonWhiteSpace(str, pos);

            if(pos == -1) {
                break;
            }

            hexDigits[hexPos] = str[pos];
            hexPos++;

            if(hexPos >= 2) {
                hexPos = 0;

                retBuf[bufPos++] =
                    (stringHexToNibble(hexDigits[0]) << 4) +
                    stringHexToNibble(hexDigits[1]);

            }

            pos++;
        }

        return retBuf;
    }


    inline void stringSplit(
        const std::string &str,
        const std::string &divider,
        std::string &out1,
        std::string &out2,
        bool startFromEnd)
    {
        out1 = out2 = "";

        // Handle degenerate cases so we don't have to worry later.
        if(!divider.size()) {
            out1 = str;
            return;
        }
        if(!str.size()) {
            return;
        }

        // ITT ternary operator abuse for clever for loops that go in
        // whatever direction we want.
        for(unsigned int i = (startFromEnd ? str.size() - 1 : 0);
            // startFromEnd logic handled separately before overflow
            // can happen.
            startFromEnd || (i < str.size());
            i += (startFromEnd ? -1 : 1)) {

            if(str[i] == divider[0]) {
                if(str.substr(i, divider.size()) == divider) {
                    // Found the division.
                    out1 = str.substr(0, i);
                    out2 = str.substr(i + divider.size());
                    return;
                }
            }

            // Unsigned value going through the string doesn't
            // actually become negative ever.
            if(startFromEnd && i == 0) {
                break;
            }
        }

        // Made it to the end of the string without finding anything.
        if(startFromEnd) {
            out2 = str;
        } else {
            out1 = str;
        }
    }

    inline bool stringParseUri(
        const std::string &input,
        std::string &outScheme,
        std::string &outAuthority,
        std::string &outPath,
        std::string &outQuery,
        std::string &outFragment)
    {
        std::string queryFragment;
        std::string schemeAuthorityPath;

        stringSplit(input, "?", schemeAuthorityPath, queryFragment);

        // Handle the case where there was no '?' but we still need to
        // check for a fragment.
        if(!queryFragment.size()) {

            // Fragment could still be at the end of schemeAuthorityPath.
            outQuery = "";
            std::string firstPart;
            stringSplit(schemeAuthorityPath, "#", firstPart, outFragment);

            // Make sure we use the now properly stripped down path.
            schemeAuthorityPath = firstPart;

        } else {

            // Fragment is after query.
            stringSplit(queryFragment, "#", outQuery, outFragment);
        }

        std::string tmp1;
        std::string tmp2;

        stringSplit(schemeAuthorityPath, ":", tmp1, tmp2);
        if(!tmp2.size()) {
            // Didn't split this anywhere, so the scheme was
            // omitted. Perhaps this should actually return an error
            // indicating an invalid URL.
            outScheme = "";
            tmp2 = tmp1;
            tmp1 = "";
        } else {
            // Scheme ended up as the first output.
            outScheme = tmp1;
        }

        // Knock out double forward slashes. (COLLADA doesn't want
        // them and Tim Berners-Lee has apologized for their
        // existence.)
        if(tmp2.size() >= 2 && tmp2[0] == '/' && tmp2[1] == '/') {
            tmp2 = tmp2.c_str() + 2;
        }

        // TODO: Figure out the distinction between absolute paths and
        // not here.

        // TODO: Missing authority okay for file:// scheme and nothing
        // else?

        // tmp2 now has the authority and path.

        if(outScheme == "file" || !outScheme.size()) {
            // Probably a file path.
            outPath = tmp2;
        } else {
            // Otherwise, handle normally.
            stringSplit(tmp2, "/", outAuthority, outPath);
            outPath = "/" + outPath;
        }

        return true;
    }

    // Apparently "0b01010101" is not valid syntax, so here's a bunch of
    // stuff to help us keep track of bits easier in UTF-8 decoding.
    const uint8_t EXPOP_b10000000 = 128;
    const uint8_t EXPOP_b01000000 = 64;
    const uint8_t EXPOP_b00111111 = 63;

    // TODO: Handle byte order marker.
    inline std::basic_string<uint32_t> stringUTF8ToUTF32(
        const std::string &utf8Str) {

        std::basic_string<uint32_t> utf32Out;

        // TODO: Recognize and discard the UTF-8 byte order mark. (The
        // only byte order mark that even means anything in UTF-8.)

        for(unsigned int i = 0; i < utf8Str.size(); i++) {

            unsigned char inByte = utf8Str[i];
            unsigned int validBitsInFirstByte = 7;
            unsigned int numExtraBytes = 0;

            if(inByte & EXPOP_b10000000) {

                // This is the start of some extended character because it
                // has a 1 in the hi bit.

                int checkBit = EXPOP_b01000000;

                // We go from 7 bits to 5 bits in the first byte just
                // because it's an extended thing.
                validBitsInFirstByte--;

                while(inByte & checkBit) {
                    checkBit >>= 1;
                    validBitsInFirstByte--;
                    numExtraBytes++;
                }
            }

            // (1 << validBitsInFirstByte) - 1 should turn into a mask of
            // bits that we can use on the first byte.
            uint32_t outCharacter = inByte & ((1 << validBitsInFirstByte) - 1);

            for(unsigned int j = 0; j < numExtraBytes; j++) {

                i++;
                if(i >= utf8Str.size()) break;

                outCharacter <<= 6;
                outCharacter |= utf8Str[i] & EXPOP_b00111111;
            }

            utf32Out.push_back(outCharacter);
        }

        return utf32Out;
    }

    inline int findHighestBit(unsigned int bits) {
        for(int i = sizeof(bits) * 8 - 1; i >= 0; i--) {
            if(bits & (1 << i)) {
                return i;
            }
        }
        return -1;
    }

    // // These are utility functions that we'll need if we ever want
    // // to debug the stringUTF8ToUTF32 or stringUTF32ToUTF8 functions.

    // static std::string makeBitsString(unsigned int bits, int maxBits = -1) {
    //     ostringstream str;
    //     if(maxBits == -1) maxBits = sizeof(bits) * 8;
    //     for(int i = maxBits - 1; i >= 0; i--) {
    //         if(bits & (1 << i)) {
    //             str << "1";
    //         } else {
    //             str << "0";
    //         }
    //     }
    //     return str.str();
    // }

    // static std::string makeHexString(unsigned int bits, int maxBytes = -1) {
    //     ostringstream str;
    //     if(maxBytes == -1) maxBytes = sizeof(bits);
    //     for(int i = 0; i < maxBytes; i++) {
    //         str << nibbleToHex((bits >> (i * 8 + 4)) & 0x0F);
    //         str << nibbleToHex((bits >> (i * 8)) & 0x0F);
    //     }
    //     return str.str();
    // }

    inline std::string stringUTF32ToUTF8(
        const std::vector<uint32_t> &utf32Str)
    {
        std::ostringstream outStr;

        // TODO: Recognize and use the byte order marks if they're
        // present.

        // This will store a chunk that we'll drop into outStr all at
        // once. Beware that it can't store null terminators in it
        // because it'll chop off the remaining data if we try.  It's
        // only used in the multi-byte character output, and all of
        // those have a 1 in the high bit, so it'll never be zero.
        char tmpOutChunk[16];

        for(unsigned int i = 0; i < utf32Str.size(); i++) {

            int highestBit = findHighestBit(utf32Str[i]);

            // Figure out how many bytes we're going to need. Six bits
            // per byte except for the first byte, which can hold a
            // variable number depending on how many other bytes are
            // being used. The 'f' variable is the space available in
            // the first byte for data. Starts at 5, then goes down
            // for every extra byte.
            int numBytesNeeded = 1;
            if(highestBit > 6) {
                int f = 5;
                int c = highestBit;
                while(c >= 0) {
                    c -= 6 + f;
                    f--;
                    numBytesNeeded++;
                }
            }

            if(numBytesNeeded > 1) {

                // This is a multi-byte character of some sort.

                // First, copy the uniformly sized chunks over. Each
                // of these bytes are data for the low six bits and
                // just 10 for the two highest bits. Note that it
                // copies starting from the end of the array and
                // moving towards the front.
                int numFullBytes = numBytesNeeded - 1;
                uint32_t outByte = 0;
                for(int fb = numFullBytes; fb > 0; fb--) {

                    // Try not to think about this line too hard.
                    outByte = 0x80 | ((utf32Str[i] >> (6 * (numFullBytes - fb))) & (EXPOP_b00111111));

                    tmpOutChunk[fb] = char(outByte);

                }

                // Now the first byte in the array is going to be
                // different. The first few bits of it are 1 for each
                // byte that was needed including the first.
                uint32_t currentBitNum = 0;
                outByte = 0;
                for(currentBitNum = 0; currentBitNum < (uint32_t)numBytesNeeded; currentBitNum++) {
                    outByte |= (0x80 >> currentBitNum);
                }

                uint32_t finalDataBitMask = (0x80 >> currentBitNum) - 1;

                outByte |= (utf32Str[i] >> (6 * numFullBytes)) & finalDataBitMask;

                tmpOutChunk[0] = outByte;

                // Add the real null terminator.
                assert(numBytesNeeded < 15);
                tmpOutChunk[numBytesNeeded] = 0;

                outStr << tmpOutChunk;

            } else {

                // ASCII character. Just copy it in.
                outStr << char(utf32Str[i]);
            }
        }

        return outStr.str();
    }

    // FIXME: Not UTF-8 compatible.
    inline std::string stringWordWrap(
        const std::string &str,
        unsigned int columns,
        unsigned int columnsAfterFirstLine)
    {
        std::vector<std::string> lines;
        std::vector<std::string> words;
        std::ostringstream outStr;
        unsigned int thisLineLength = 0;
        bool firstLine = true;
        bool firstWordOnLine = true;

        if(!columnsAfterFirstLine) columnsAfterFirstLine = columns;

        stringTokenize(str, " \t\r\n", words, false);

        for(unsigned int i = 0; i < words.size(); i++) {

            // New line?
            if(thisLineLength != 0 && words[i].size() + thisLineLength >= (firstLine ? columns : columnsAfterFirstLine)) {
                outStr << std::endl;
                thisLineLength = 0;
                firstLine = false;
                firstWordOnLine = true;
            }

            // Avoid trailing whitespace and whitespace at the start
            // of a line. Don't put the space before this word if it's
            // the first one on a line.
            if(!firstWordOnLine) {
                outStr << " ";
                thisLineLength++;
            } else {
                firstWordOnLine = false;
            }

            // Add the word.
            outStr << words[i];
            thisLineLength += words[i].size();
        }

        return outStr.str();
    }

    inline std::string stringIndent(
        const std::string &str,
        unsigned int firstRow,
        unsigned int afterFirstRow)
    {
        std::vector<std::string> lines;
        std::ostringstream outStr;

        stringTokenize(str, "\n", lines);

        for(size_t i = 0; i < lines.size(); i++) {

            // Skip past whatever whitespace might have been here in
            // the first place.
            unsigned int j = 0;
            while(j < lines[i].size() && isWhiteSpace(lines[i][j])) {
                j++;
            }

            // Output a newline if this isn't the first line. We do it
            // here so we don't end up with some extra newline at the
            // end.
            if(i != 0) outStr << std::endl;

            if(j < lines[i].size()) {

                // FIXME: Generate strings for first line and after
                // first line indentation, then use one or the other
                // here.

                // Output the desired indentation.
                unsigned int dstLen = (i == 0) ? firstRow : afterFirstRow;
                for(unsigned int k = 0; k < dstLen; k++) {
                    outStr << " ";
                }

                outStr << lines[i].substr(j);

            }
        }

        return outStr.str();
    }

    inline std::string stringPrefixLines(
        const std::string &str,
        const std::string &prefix)
    {
        std::ostringstream outStr;
        std::vector<std::string> lines;

        stringTokenize(str, "\n", lines, true);

        for(unsigned int i = 0; i < lines.size(); i++) {

            outStr << prefix << lines[i];

            // Add a newline if this isn't the last one. Avoids
            // trailing newline.
            if(i != lines.size() - 1) {
                outStr << std::endl;
            }
        }

        return outStr.str();
    }

    template<typename T>
    inline std::basic_string<T> stringToLower(
        const std::basic_string<T> &str)
    {
        std::string ret;
        ret = str;
        for(size_t i = 0; i < ret.size(); i++) {
            ret[i] = tolower(ret[i]);
        }
        return ret;
    }
}

