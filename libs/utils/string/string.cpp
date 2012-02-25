// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2010 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   Copyright (c) 2011 Clifford Jolly
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

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

#include <cassert>

#include <malloc.h>

#include "malstring.h"

namespace ExPop {

    bool strStartsWith(const std::string &needle, const std::string &haystack) {
        if(haystack.size() < needle.size()) return false;
        return strncmp(needle.c_str(), haystack.c_str(), needle.size()) == 0;
    }

    bool strEndsWith(const std::string &needle, const std::string &haystack) {
        if(haystack.size() < needle.size()) return false;
        return strncmp(needle.c_str(), haystack.c_str() + (haystack.size() - needle.size()), needle.size()) == 0;
    }

    bool isWhiteSpace(char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    void stringTokenize(const std::string &str, const std::string &delims, std::vector<std::string> &tokens, bool allowEmpty) {

        unsigned int i = 0;
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
                string token = str.substr(tokStart, (tokEnd - tokStart));
                tokens.push_back(token);
            }
        }
    }

    std::string stringEscape(const std::string &str, const std::string &newLineReplace) {

        ostringstream outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            switch(str[i]) {

                case '"':
                    outStr << "\\\"";
                    break;

                case '\\':
                    outStr << "\\\\";
                    break;

                case '\n':
                    outStr << newLineReplace;
                    break;

                default:
                    outStr << str[i];
                    break;
            }

        }

        return outStr.str();

    }

    std::string stringUnescape(const std::string &str) {

        ostringstream outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            if(str[i] == '\\') {

                i++;

                switch(str[i]) {

                    case '\\':
                        outStr << "\\";
                        break;

                    case '"':
                        outStr << "\"";
                        break;

                    case 'n':
                        outStr << "\n";
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

    static inline int getNextNonWhiteSpace(const std::string &str, int start) {

        if(start >= int(str.size())) return -1;

        while(isWhiteSpace(str[start])) {
            start++;
            if(start >= int(str.size())) return -1;
        }

        return start;
    }


    static inline char nibbleToHex(unsigned int n) {

        if(n < 0xa) {
            return '0' + n;
        } else {
            return ('a' + n) - 0xa;
        }

    }

    static inline unsigned int hexToNibble(char h) {

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


    void strEncodeHex(const void *buf, int length, string &str, int columns) {

        str.resize((length * 2) + ((length * 2) / columns), 0);

        int strPos = 0;
        int lineCounter = 0;
        const char *cbuf = (const char *)buf;

        for(int i = 0; i < length; i++) {
            str[strPos++] = nibbleToHex((cbuf[i] & 0xf0) >> 4);
            str[strPos++] = nibbleToHex((cbuf[i] & 0x0f));

            lineCounter += 2;

            if(lineCounter > columns) {
                str[strPos++] = '\n';
                lineCounter = 0;
            }
        }

        str.resize(strPos);

    }

    char *strDecodeHex(const std::string &str, int *length) {

        // Do this in two passes. First, count up the number of bytes.
        int pos = 0;

        int hexPos = 0;
        char hexDigits[2];
        int numBytes = 0;

        while(pos < int(str.size())) {
            pos = getNextNonWhiteSpace(str, pos);

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
            pos = getNextNonWhiteSpace(str, pos);

            if(pos == -1) {
                break;
            }

            hexDigits[hexPos] = str[pos];
            hexPos++;

            if(hexPos >= 2) {
                hexPos = 0;

                retBuf[bufPos++] =
                    (hexToNibble(hexDigits[0]) << 4) +
                    hexToNibble(hexDigits[1]);

            }

            pos++;
        }

        return retBuf;

    }


    void stringSplit(
        const std::string &str,
        const std::string &divider,
        std::string &out1,
        std::string &out2,
        bool startFromEnd) {

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
            // FIXME: i >= 0 test doesn't actually do anything.
            (startFromEnd ? (i >= 0) : (i < str.size()));
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
        out1 = str;
    }

    bool parseUri(
        const std::string &input,
        std::string &outScheme,
        std::string &outAuthority,
        std::string &outPath,
        std::string &outQuery,
        std::string &outFragment) {

        string queryFragment;
        string schemeAuthorityPath;

        stringSplit(input, "?", schemeAuthorityPath, queryFragment);

        // Handle the case where there was no '?' but we still need to
        // check for a fragment.
        if(!queryFragment.size()) {

            // Fragment could still be at the end of schemeAuthorityPath.
            outQuery = "";
            string firstPart;
            stringSplit(schemeAuthorityPath, "#", firstPart, outFragment);

            // Make sure we use the now properly stripped down path.
            schemeAuthorityPath = firstPart;

        } else {

            // Fragment is after query.
            stringSplit(queryFragment, "#", outQuery, outFragment);
        }

        string tmp1;
        string tmp2;

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
#define b10000000 128
#define b01000000 64
#define b00111111 63

    void strUTF8ToUTF32(
        const std::string &utf8Str,
        std::vector<unsigned int> &utf32Out) {

        for(unsigned int i = 0; i < utf8Str.size(); i++) {

            unsigned char inByte = utf8Str[i];
            unsigned int validBitsInFirstByte = 7;
            unsigned int numExtraBytes = 0;

            if(inByte & b10000000) {

                // This is the start of some extended character because it
                // has a 1 in the hi bit.

                int checkBit = b01000000;

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
            unsigned int outCharacter = inByte & ((1 << validBitsInFirstByte) - 1);

            for(unsigned int j = 0; j < numExtraBytes; j++) {

                i++;
                if(i >= utf8Str.size()) break;

                outCharacter <<= 6;
                outCharacter |= utf8Str[i] & b00111111;
            }

            utf32Out.push_back(outCharacter);
        }

    }

    int strToConst(
        const std::string &str,
        const StringToConstMapping *table,
        const std::string &fieldName,
        std::ostream *errorOut) {

        // Try to find whatever value in the table.

        int i;
        for(i = 0; strlen(table[i].str); i++) {
            if(str == table[i].str) {
                return table[i].val;
            }
        }

        if(str.size() && errorOut) {

            // Tried to have a value, but didn't find a match. Show an
            // error. If the user didn't specify a value we'd just go
            // with the default without complaining.

            (*errorOut) << "Unknown value (" << str << ") for field " << fieldName << endl;
            (*errorOut) << "Possible values are..." << endl;

            for(int j = 0; strlen(table[j].str); j++) {
                (*errorOut) << "  " << table[j].str << endl;
            }
        }

        // TODO: Possibly fill up a table of autocomplete
        // possibilities for whatever we got as input.

        // i is still at the final, default value, so just go ahead
        // and return that.
        return table[i].val;

    }

    // ----------------------------------------------------------------------
    //   SimpleBuffer stuff after this point.
    // ----------------------------------------------------------------------

    SimpleBuffer::SimpleBuffer(const void *data, int length, bool myOwnData) {

        this->myOwnData = myOwnData;

        this->length = 0;

        if(myOwnData) {

            // This object keeps its own copy of the data.
            this->data = (char*)malloc(length);
            this->length = length;
            memcpy(this->data, data, length);

        } else {

            // Not my own data.

            // FIXME: This just gets rid of the const. Probably a really
            // horrible way to handle this, but whatever.
            memcpy(&this->data, &data, sizeof(void*));
            this->length = length;
        }

        readPtr = 0;
    }

    SimpleBuffer::SimpleBuffer(void) {

        myOwnData = true;

        data = NULL;
        length = 0;

        readPtr = 0;
    }

    SimpleBuffer::~SimpleBuffer(void) {
        if(myOwnData && data) free(data);
    }

    void SimpleBuffer::clear(void) {
        if(myOwnData && data) free(data);
        data = NULL;
    }

    void SimpleBuffer::addData(const void *buffer, int length) {

        assert(myOwnData);

        int newLength = length + this->length;
        int oldLength = this->length;

        if(data) {
            data = (char*)realloc(data, newLength);
        } else {
            data = (char*)malloc(newLength);
        }

        char *dataWritePtr = data + oldLength;

        memcpy(dataWritePtr, buffer, length);

        this->length = newLength;
    }

    const char *SimpleBuffer::getData(void) const {
        return data;
    }

    const char *SimpleBuffer::getDataAndAdvance(void *dst, int bytes) {

        if(readPtr >= length) return NULL;

        const char *ret = data + readPtr;

        if(dst) {

            int realBytes = bytes;

            if(readPtr + bytes > length) {
                // Trying to run off the end of the buffer.
                realBytes = length - readPtr;
            }

            if(realBytes != bytes) {
                // If we would go off the end of the buffer, pad the ouput
                // with stuff we can't do.
                memset(dst, 0, bytes);
            }

            memcpy(dst, ret, realBytes);
        }

        readPtr += bytes;

        return ret;
    }

    void SimpleBuffer::seekToStart(void) {
        readPtr = 0;
    }

    int SimpleBuffer::getLength(void) const {
        return length;
    }

    int SimpleBuffer::compare(const void *buffer, int length) {

        if(length < this->length) return 1;
        if(length > this->length) return -1;

        const char *cbuf = (const char *)buffer;

        for(int i = 0; i < length; i++) {
            if(data[i] > cbuf[i]) {
                return 1;
            }
            if(data[i] < cbuf[i]) {
                return -1;
            }
        }

        return 0;
    }

};

