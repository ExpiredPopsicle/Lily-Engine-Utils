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

#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace ExPop {

    /// Return true if the character is whitespace. False otherwise.
    bool isWhiteSpace(char c);

    /// Simple function to see if a string starts with another string.
    /// (needle is the string we're looking for, haystack is the
    /// string we're looking in).
    bool strStartsWith(const std::string &needle, const std::string &haystack);

    /// Return true if one string ends with another string. (needle is
    /// the string we're looking for, haystack is the string we're
    /// looking in).
    bool strEndsWith(const std::string &needle, const std::string &haystack);

    /// Generate tokens from a string. Stores saved tokens into the
    /// passed-in tokens parameter.
    void stringTokenize(const std::string &str, const std::string &delims, std::vector<std::string> &tokens, bool allowEmpty = false);

    // TODO: Advanced string tokenization, escape/unescape, etc.

    /// Escape a string. newLineReplace will be used to replace new
    /// lines.
    std::string stringEscape(const std::string &str, const std::string &newLineReplace = "\\n");

    /// Unescape a string.
    std::string stringUnescape(const std::string &str);

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

    /// Convert a UTF-8 string to a UTF-32 string (represented as an
    /// std::vector fo unsigned integers).
    void strUTF8ToUTF32(
        const std::string &utf8Str,
        std::vector<unsigned int> &utf32Out);

    /// Pass in arrays of this to strToConst or constToStr. Last one
    /// is a default entry with a zero-length string.
    struct StringToConstMapping {
        const char *str;
        int val;
    };

    /// Look up a string in a table of string to constant conversions.
    int strToConst(
        const std::string &str,
        const StringToConstMapping *table,
        const std::string &fieldName = "unknown",
        std::ostream *errorOut = &(std::cout));

    // TODO: Make a constToStr

    /// Return a string with all the trailing and leading whitespace
    /// removed.
    std::string strTrim(const std::string &str);

    /// Simple buffer that we can dynamically expand or iterate through.
    class SimpleBuffer {
	public:

		SimpleBuffer(void);
		SimpleBuffer(const void *data, int length, bool myOwnData = true);
		~SimpleBuffer(void);

        /// Add some data onto the end. Reallocates if needed. Note:
        /// Doesn't just double the size of the buffer. It only
        /// allocates as much as it needs so continuously adding to
        /// the buffer in this way can be slow.
		void addData(const void *buffer, int length);

        /// Just get a pointer to the start of the data.
		const char *getData(void) const;

        /// Get the current length of the data.
		int getLength(void) const;

        /// Reads a number of bytes equal to the bytes parameter into
        /// the memory pointed to by dst and advances the read pointer
        /// by that amount. Returns a pointer to the position the read
        /// pointer ends at.
		const char *getDataAndAdvance(void *dst, int bytes);

        /// Moves the read pointer back to the start of the data.
        void seekToStart(void);

        int compare(const void *buffer, int length);

        void clear(void);

	private:

        // If this is true, the data will be cleaned up when this goes
        // away.
        bool myOwnData;

		char *data;
		int length;
		int readPtr;
    };

};

