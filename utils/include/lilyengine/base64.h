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

// Simple base64 conversions.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <string>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Decode a buffer from a base64 string as specified in RFC 2045.
    /// Returns a pointer to a buffer allocated with new.
    /// Responsibility for freeing this buffer with delete[] is up to
    /// the caller.
    inline unsigned char *stringBase64Decode(
        const std::string &str, unsigned int *length);

    /// Decode a string from a base64 string. Convenience function for
    /// text data.
    inline std::string stringBase64DecodeString(const std::string &str);

    /// Encode a buffer to a base64 string.
    inline std::string stringBase64Encode(const void *buffer, size_t length);

    /// Encode a string to a base64 string. Convenience function for
    /// text data.
    inline std::string stringBase64EncodeString(const std::string &str);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline uint32_t stringBase64DecodeLetter(char c)
    {
        if(c >= 'A' && c <= 'Z') {
            return c - 'A';
        }

        if(c >= 'a' && c <= 'z') {
            return (c - 'a') + 26;
        }

        if(c >= '0' && c <= '9') {
            return (c - '0') + 52;
        }

        if(c == '+') {
            return 62;
        }

        if(c == '/') {
            return 63;
        }

        if(c == '=') {
            return 0;
        }

        return 0;
    }

    inline uint32_t stringBase64DecodeChunk(const std::string &chunk)
    {
        assert(chunk.size() == 4);
        return
            (stringBase64DecodeLetter(chunk[0]) << 18) |
            (stringBase64DecodeLetter(chunk[1]) << 12) |
            (stringBase64DecodeLetter(chunk[2]) << 6) |
            (stringBase64DecodeLetter(chunk[3]));
    }

    inline unsigned char *stringBase64Decode(const std::string &str, unsigned int *length)
    {
        // These things are always 4-byte padded, so if it's not that,
        // then we know it's invalid. Also, the algorithm would read off
        // the end of the buffer in horrible ways, so just fail.
        if(str.size() % 4) {
            return NULL;
        }

        // Output buffer is 3 bytes per 4-character chunk. Each 4
        // characters represents 24 bits.
        size_t bufSize = (str.size() / 4) * 3;

        // Adjust the buffer size down based on the padding at the end of
        // the input string.
        if(str.size() >= 3) {

            // No = - Last chunk is 24 bits.
            // ==   - Last chunk is 8 bits.
            // =    - Last chunk is 16 bits.

            // Note: One '=' does not really correspond to one output
            // byte, but the cases are all the same as if they did. Don't
            // get confused between the encoded 6-bit digits and unencoded
            // 8-bit digits!
            if(str[str.size() - 1] == '=') bufSize--;
            if(str[str.size() - 2] == '=') bufSize--;
        }

        unsigned char *buffer = new unsigned char[bufSize];
        unsigned int dataPtr = 0;

        // Iterate through the four character chunks of the string and
        // decode them.
        for(unsigned int i = 0; i < str.size(); i += 4) {

            uint32_t t = stringBase64DecodeChunk(str.substr(i, 4));

            // Write the bytes into the buffer from the decoded 24-bits.
            for(unsigned int k = 0; k < 3; k++) {
                if(dataPtr < bufSize) {
                    buffer[dataPtr] = (t >> ((2-k)*8)) & 0xff;
                    dataPtr++;
                }
            }
        }

        *length = bufSize;
        return buffer;
    }

    inline std::string stringBase64DecodeString(const std::string &str)
    {
        unsigned int length = 0;
        unsigned char *buf = stringBase64Decode(str, &length);

        std::string ret((const char*)buf, length);
        delete[] buf;
        return ret;
    }

    inline uint32_t stringBase64Encode_getNextChunk(unsigned char *&ptr, size_t &length)
    {
        uint32_t ret = 0;

        for(unsigned int i = 0; i < 3; i++) {

            ret |= (uint32_t(*(ptr))) << ((2-i)*8);
            ptr++;
            length--;

            if(!length) break;
        }

        return ret;
    }

    inline std::string stringBase64Encode(const void *buffer, size_t length)
    {
        unsigned char *ptr = (unsigned char*)buffer;

        std::ostringstream ostr;

        static const char strBase64Lookup[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789"
            "+/";

        size_t tmpLength = length;

        while(tmpLength) {
            std::ostringstream currentChunkStr;

            uint32_t nextChunk = stringBase64Encode_getNextChunk(ptr, tmpLength);

            for(unsigned int i = 0; i < 4; i++) {
                uint32_t chunkPiece = nextChunk >> (6 * (3-i));
                chunkPiece &= 63;
                currentChunkStr << strBase64Lookup[chunkPiece];
            }

            ostr << currentChunkStr.str();
        }

        std::string ret = ostr.str();

        // We didn't handle the '=' padding during the encoding for
        // simplicity and just let the padded area encode as zero. Go back
        // and fix that up now.
        unsigned int lm3 = length % 3;
        if(lm3 == 1) {
            ret[ret.size() - 1] = '=';
            ret[ret.size() - 2] = '=';
        } else if(lm3 == 2) {
            ret[ret.size() - 1] = '=';
        }

        return ret;
    }

    inline std::string stringBase64EncodeString(const std::string &str)
    {
        return stringBase64Encode(str.c_str(), str.size());
    }
}

