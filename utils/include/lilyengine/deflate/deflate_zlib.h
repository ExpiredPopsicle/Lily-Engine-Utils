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

// Zlib decompression system. This wraps up the other DEFLATE code,
// but reads the Zlib header and checksum footer for decompressing
// stuff compressed using Zlib's default settings.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "deflate.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        /// Decompress a chunk of data (buffer) that has been DEFLATEd
        /// using zlib, containing the zlib header and footer
        /// (checksum), and append it to outputBuf.
        ///
        /// Use start to indicate a byte offset inside the buffer. Use
        /// endPtr to save the offset of the byte after the end of the
        /// zlib footer in the stream.
        bool decompress_zlib(
            const std::string &buffer,
            std::string &outputBuf,
            size_t start = 0,
            size_t *endPtr = nullptr);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        struct ZlibHeader
        {
            union
            {
                // CMF is Compression Method and Flags. (RFC1950)
                struct
                {
                    // Bits 0 to 3 are compression method (CM). Should
                    // be 8 for DEFLATE.
                    uint8_t method:4;

                    // Bits 4 to 7 are compression info (CINFO). We
                    // can ignore it for decompression, but we should
                    // have something accurate there for compression.
                    uint8_t info:4;

                } cmf;

                uint8_t cmfByte;
            };

            union
            {
                struct
                {
                    // Sanity check value.
                    uint8_t fcheck:5;

                    // We don't really know how to deal with
                    // dictionaries right now.
                    uint8_t fdict:1;

                    // Compression level. Can also ignore this. The
                    // DEFLATE stream will specify a cmpression type
                    // regardless.
                    uint8_t flevel:2;

                } flg;

                uint8_t flgByte;
            };
        };

        inline uint32_t adler32(const std::string &in)
        {
            uint32_t a = 1, b = 0;
            size_t index;

            for(index = 0; index < in.size(); index++) {
                a = (a + (uint8_t)in[index]) % 65521;
                b = (b + a) % 65521;
            }

            return (b << 16) | a;
        }

        inline bool decompress_zlib(
            const std::string &buffer,
            std::string &outputBuf,
            size_t start,
            size_t *endPtr)
        {
            // FIXME: This should be altered to work with streams
            // instead of just buffers of data.

            static_assert(
                sizeof(ZlibHeader) == 2,
                "Zlib header struct must be two bytes.");

            // Make sure there's at least room for the zlib header.
            if(buffer.size() < 2 + start) {
                return false;
            }

            // Read ZLib header.
            ZlibHeader header;
            memcpy(&header, &buffer[start], sizeof(header));

            // Compression method 8 is DEFLATE.
            if(header.cmf.method != 8) {
                // Can only do DEFLATE.
                return false;
            }

            // 5 extra "fcheck" bits inside FLG should make this mod 31
            // = 0.
            uint16_t checkVal =
                (uint16_t(header.cmfByte) << 8) |
                uint16_t(header.flgByte);

            if((checkVal % 31) != 0) {
                // Header check failure.
                return false;
            }

            size_t startOffset = start + sizeof(header);

            // Do whatever with the dictionary.
            if(header.flg.fdict) {

                // FIXME: We should actually have some implementation
                // here. Right now we'll just skip the dictionary.
                startOffset += 4;

                return false;
            }

            // Run the inner decompression.
            size_t end = 0;
            if(!decompress(
                    buffer, outputBuf,
                    startOffset, &end))
            {
                // Some general decompression failure.
                return false;
            }

            if(end + 4 > buffer.size()) {

                // Not enough data left for the expected Adler32.
                return false;
            }

            // Check the Adler32.
            uint32_t computedAdler32 = adler32(outputBuf);
            uint32_t savedAdler32 = 0;
            memcpy(&savedAdler32, &buffer[end], sizeof(uint32_t));

            // Reverse the big-endian Adler32 value for little-endian
            // systems.
            if(systemIsLittleEndian()) {
                savedAdler32 = reverseByteOrder(savedAdler32);
            }

            if(computedAdler32 != savedAdler32) {
                // Checksum fail.
                return false;
            }

            // Skip the Adler32 and write the end pointer to point at
            // the first byte after it.
            if(endPtr) {
                *endPtr = end + sizeof(uint32_t);
            }

            return true;
        }
    }
}

