// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
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

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
using namespace std;

#include <cassert>

#include <malloc.h>

#include "malstring.h"

namespace ExPop {

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

    std::string stringXmlEscape(const std::string &str) {

        ostringstream outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

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

    std::string stringXmlUnescape(const std::string &str) {

        ostringstream outStr;

        for(unsigned int i = 0; i < str.size(); i++) {

            if(str[i] == '&') {

                i++;

                if(i >= str.size()) break;

                if(strStartsWith<char>("amp;", str.c_str() + i)) {

                    i += 3;
                    outStr << '&';

                } else if(strStartsWith<char>("quot;", str.c_str() + i)) {

                    i += 4;
                    outStr << '"';

                } else if(strStartsWith<char>("lt;", str.c_str() + i)) {

                    i += 2;
                    outStr << '<';

                } else if(strStartsWith<char>("gt;", str.c_str() + i)) {

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
                    ostringstream hexStr;
                    while(i < str.size() && str[i] != ';' && (
                          (str[i] >= '0' && str[i] <= '9') ||
                          (str[i] >= 'a' && str[i] <= 'f') ||
                          (str[i] >= 'A' && str[i] <= 'F'))) {

                        hexStr << str[i];
                        i++;

                    }

                    unsigned int hexVal =
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

    // TODO: Handle byte order marker.
    void strUTF8ToUTF32(
        const std::string &utf8Str,
        std::vector<unsigned int> &utf32Out) {

        utf32Out.clear();

        // TODO: Recognize and discard the UTF-8 byte order mark. (The
        // only byte order mark that even means anything in UTF-8.)

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

    static int findHighestBit(unsigned int bits) {
        for(int i = sizeof(bits) * 8 - 1; i >= 0; i--) {
            if(bits & (1 << i)) {
                return i;
            }
        }
        return -1;
    }

    // // These are utility functions that we'll need if we ever want
    // // to debug the strUTF8ToUTF32 or strUTF32ToUTF8 functions.

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

    void strUTF32ToUTF8(
        const std::vector<unsigned int> &utf32Str,
        std::string &utf8Out) {

        ostringstream outStr;

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
                unsigned int outByte = 0;
                for(int fb = numFullBytes; fb > 0; fb--) {

                    outByte = 0x80 | ((utf32Str[i] >> (6 * (numFullBytes - fb))) & (b00111111));

                    tmpOutChunk[fb] = char(outByte);

                }

                // Now the first byte in the array is going to be
                // different. The first few bits of it are 1 for each
                // byte that was needed including the first.
                unsigned int currentBitNum = 0;
                outByte = 0;
                for(currentBitNum = 0; currentBitNum < (unsigned int)numBytesNeeded; currentBitNum++) {
                    outByte |= (0x80 >> currentBitNum);
                }

                unsigned int finalDataBitMask = (0x80 >> currentBitNum) - 1;

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

        // Done. Now just copy out the string.
        utf8Out = outStr.str();
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

    std::string constToStr(
        int value,
        const StringToConstMapping *table) {

        int i;
        for(i = 0; strlen(table[i].str); i++) {
            if(table[i].val == value) {
                return table[i].str;
            }
        }

        return "";
    }

    std::string strTrim(const std::string &str) {

        if(!str.size()) return "";

        unsigned int start = 0;

        // Find the start.
        while(start < str.size() && isWhiteSpace(str[start])) {
            start++;
        }

        // All whitespace? Return now. Otherwise we'll end up with
        // some serious issues when the end is the start and the start
        // is the end.
        if(start == str.size()) return "";

        // Find the end.
        unsigned int end = str.size() - 1;
        while(1) {

            if(!isWhiteSpace(str[end])) {
                break;
            }

            if(end == 0) break;
            end--;
        }

        return str.substr(start, end - start + 1);
    }

    // FIXME: Not UTF-8 compatible.
    std::string strWordWrap(
        const std::string &str,
        unsigned int columns,
        unsigned int columnsAfterFirstLine) {

        vector<string> lines;
        vector<string> words;
        ostringstream outStr;
        unsigned int thisLineLength = 0;
        bool firstLine = true;
        bool firstWordOnLine = true;

        if(!columnsAfterFirstLine) columnsAfterFirstLine = columns;

        stringTokenize(str, " \t\r\n", words, false);

        for(unsigned int i = 0; i < words.size(); i++) {

            // New line?
            if(thisLineLength != 0 && words[i].size() + thisLineLength >= (firstLine ? columns : columnsAfterFirstLine)) {
                outStr << endl;
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

    std::string strIndent(
        const std::string &str,
        unsigned int firstRow,
        unsigned int afterFirstRow) {

        vector<string> lines;
        ostringstream outStr;

        stringTokenize(str, "\n", lines);

        for(unsigned int i = 0; i < lines.size(); i++) {

            // Skip past whatever whitespace might have been here in
            // the first place.
            unsigned int j = 0;
            while(j < lines[i].size() && isWhiteSpace(lines[i][j])) {
                j++;
            }

            // Output a newline if this isn't the first line. We do it
            // here so we don't end up with some extra newline at the
            // end.
            if(i != 0) outStr << endl;

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

    std::string strPrefixLines(
        const std::string &str,
        const std::string &prefix) {

        ostringstream outStr;
        vector<string> lines;

        stringTokenize(str, "\n", lines, true);

        for(unsigned int i = 0; i < lines.size(); i++) {

            outStr << prefix << lines[i];

            // Add a newline if this isn't the last one. Avoids
            // trailing newline.
            if(i != lines.size() - 1) {
                outStr << endl;
            }
        }

        return outStr.str();
    }

    // ----------------------------------------------------------------------
    //   SimpleBuffer stuff after this point.
    // ----------------------------------------------------------------------

    SimpleBuffer::SimpleBuffer(const void *inData, size_t inLength, bool inMyOwnData) {

        this->myOwnData = inMyOwnData;

        this->length = 0;

        if(inMyOwnData) {

            // This object keeps its own copy of the data.
            this->data = (char*)malloc(inLength);
            this->length = inLength;
            memcpy(this->data, inData, inLength);

        } else {

            // Not my own data.

            // FIXME: This just gets rid of the const. Probably a really
            // horrible way to handle this, but whatever.
            memcpy(&this->data, &inData, sizeof(void*));
            this->length = inLength;
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

    void SimpleBuffer::addData(const void *buffer, size_t inLength) {

        assert(myOwnData);

        size_t newLength = inLength + this->length;
        size_t oldLength = this->length;

        if(data) {
            data = (char*)realloc(data, newLength);
        } else {
            data = (char*)malloc(newLength);
        }

        char *dataWritePtr = data + oldLength;

        memcpy(dataWritePtr, buffer, inLength);

        this->length = newLength;
    }

    const char *SimpleBuffer::getData(void) const {
        return data;
    }

    const char *SimpleBuffer::getDataAndAdvance(void *dst, size_t bytes) {

        if(readPtr >= length) return NULL;

        const char *ret = data + readPtr;

        if(dst) {

            size_t realBytes = bytes;

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

    size_t SimpleBuffer::getLength(void) const {
        return length;
    }

    int SimpleBuffer::compare(const void *inBuffer, size_t inLength) {

        if(inLength < this->length) return 1;
        if(inLength > this->length) return -1;

        const char *cbuf = (const char *)inBuffer;

        for(size_t i = 0; i < inLength; i++) {
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

