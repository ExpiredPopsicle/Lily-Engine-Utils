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

// Barely effective RLE compression. This code should probably be
// retired.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <cassert>
#include <cstring>
#include <iostream>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    // All of these compression functions kind of suck. You should
    // probably just use zlib like anyone who's sane. (They work okay
    // for Lily game tilemaps, though.)

    /// Compress a chunk of data. Compressed data will be returned and
    /// is owned by the caller (must eventually delete[] it). Length
    /// of the returned chunk is stored in outLength. groupSize is the
    /// number of bytes to be considered a single unit for the
    /// purposes of run-length encoding.
    char *compressRLE(
        const char *inData,
        unsigned int inLength,
        unsigned int *outLength,
        int groupSize = 1);

    /// Same as above, but takes a previously compressed chunk and
    /// returns a decompressed chunk. Make sure to use the same
    /// groupSize for decompression. If decompression fails due to
    /// corrupt data or incorrect groupSize, will return NULL. (Or
    /// possibly just crash. :D)
    char *decompressRLE(
        const char *inData,
        unsigned int inLength,
        unsigned int *outLength,
        int groupSize = 1);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------


namespace ExPop
{
    const size_t compressLengthBits = 15;
    const size_t MAXCHUNKLENGTH = (1 << (compressLengthBits - 1));

    // This goes at the beginning of the whole compressed block of data.
    struct CompressHeader
    {
        unsigned int fullLength;
    };

    // This precedes every chunk of bytes, which may be a variable number
    // of raw or run length bytes.
    struct CompressChunkHeader
    {
        // 0 = raw length.
        // 1 = run length.
        unsigned short rle    : 1;

        // Length of the chunk. (1-128, so add one to this.)
        unsigned short length : compressLengthBits;
    };

    inline bool compressShouldDoRunLength(const char *inData, unsigned int lengthLeft, int groupSize)
    {
        unsigned int checkLen = sizeof(CompressChunkHeader) + (groupSize * 2);

        if(lengthLeft < checkLen * groupSize) {
            // Not actually enough to justify either way.
            return false;
        }

        for(unsigned int i = 1; i < checkLen; i++) {
            for(int j = 0; j < groupSize; j++) {
                if(inData[j] != inData[j + (i * groupSize)]) {
                    return false;
                }
            }
        }

        // Next few bytes are the same. This means that space WILL be
        // saved by turning it into a run length, which will be the one
        // byte header and one byte data. Everything else is extra.
        return true;
    }

    inline bool compressNextFewBytesSame(const char *b1, const char *b2, int length, int lengthLeft)
    {
        if(length > lengthLeft) return false;

        for(int i = 0; i < length; i++) {

            if(b1[i] != b2[i]) {
                return false;
            }

        }

        return true;
    }

    inline char *compressRLE(const char *inData, unsigned int inLength, unsigned int *outLength, int groupSize)
    {
        // Worst case we end up with nothing but raw lengths. The max
        // length of that will then be the incoming length plus one chunk
        // header for every 128 bytes. (Plus one because C++ will round
        // down.) Also add the CompressHeader.
        unsigned int scratchAllocatedLength =
            (sizeof(CompressHeader) +
             inLength +
             ((groupSize * inLength / (MAXCHUNKLENGTH-1)) + 1) * sizeof(CompressChunkHeader));

        //scratchAllocatedLength *= 4;
        //scratchAllocatedLength *= groupSize;

        // We'll add this up as we go along.
        unsigned int scratchActualLength = 0;

        // Build this up and reallocate it when we're done.
        char *scratchBuffer = new char[scratchAllocatedLength];
        assert(scratchBuffer);

        unsigned int pos = 0;

        CompressHeader *mainHeader = (CompressHeader*)scratchBuffer;
        scratchActualLength += sizeof(CompressHeader);

        while(pos < inLength) {

            bool doRLE = compressShouldDoRunLength(inData + pos, inLength - pos, groupSize);

            if(doRLE) {

                char *runGroup = new char[groupSize];

                assert(scratchActualLength < scratchAllocatedLength);

                CompressChunkHeader *header = (CompressChunkHeader*)&(scratchBuffer[scratchActualLength]);
                scratchActualLength += sizeof(CompressChunkHeader);

                header->rle = 1;
                header->length = 0; // Zero actually means 1 here. For the first byte we got at runByte.

                // Record the bytes.
                for(int i = 0; i < groupSize; i++) {
                    runGroup[i] = inData[pos++];
                    scratchBuffer[scratchActualLength++] = runGroup[i];
                }

                while(compressNextFewBytesSame(runGroup, inData + pos, groupSize, inLength - pos)) {

                    pos += groupSize;
                    header->length++;

                    if(header->length >= (MAXCHUNKLENGTH-1)) {

                        // Reached the max length we can express.
                        break;
                    }

                    if(pos >= inLength) {

                        // Reached the end of data to compress.
                        break;
                    }

                }

                delete[] runGroup;

            } else {

                CompressChunkHeader *header = (CompressChunkHeader*)&scratchBuffer[scratchActualLength];
                scratchActualLength += sizeof(CompressChunkHeader);

                header->rle = 0;
                header->length = 0; // This will be the unsigned equivalent of -1.

                while(!compressShouldDoRunLength(inData + pos, inLength - pos, groupSize)) {

                    for(int i = 0; i < groupSize && pos < inLength; i++) {
                        scratchBuffer[scratchActualLength++] = inData[pos++];
                    }

                    header->length++;

                    if(header->length >= (MAXCHUNKLENGTH-1)) {

                        // Reached the max length we can express.
                        break;
                    }

                    if(pos >= inLength) {

                        // Reached the end of data to compress.
                        break;
                    }
                }

            }

        }

        // Final length adjustments.
        *outLength = scratchActualLength;

        mainHeader->fullLength = inLength;

        // Reallocate scratchbuffer as something smaller.
        char *outBuffer = new char[scratchActualLength];
        memcpy(outBuffer, scratchBuffer, scratchActualLength);
        delete[] scratchBuffer;
        return outBuffer;

    }

    inline char *decompressRLE(const char *inData, unsigned int inLength, unsigned int *outLength, int groupSize)
    {
        if(inLength < sizeof(CompressHeader)) {

            // No way that's valid.
            *outLength = 0;
            return NULL;
        }

        unsigned int pos = 0;       // Compressed buffer position.
        unsigned int decompPos = 0; // Decompression buffer position.

        CompressHeader *mainHeader = (CompressHeader*)inData;
        pos += sizeof(CompressHeader);

        char *decompressed = new char[mainHeader->fullLength];
        *outLength = mainHeader->fullLength;

        while(pos < inLength && decompPos < *outLength) {

            // Next header.
            CompressChunkHeader *header = (CompressChunkHeader*)&inData[pos];
            pos += sizeof(CompressChunkHeader);

            if(pos >= inLength) break;

            // Pull out the run length and adjust it. (0 = 1, etc.)
            unsigned int realLength = header->length;

            if(header->rle) {

                // Tun length. Copy the same byte group over and over until
                // its length runs out.

                const char *runGroup = inData + pos;

                pos += groupSize;

                // I have no idea why I have to do this.
                realLength++;

                while(realLength) {

                    for(int i = 0; i < groupSize && decompPos < *outLength; i++) {
                        decompressed[decompPos++] = runGroup[i];
                    }

                    realLength--;
                }

            } else {

                // Raw length. Copy bytes directly from the stream until
                // its length runs out.

                while(realLength) {

                    const char *rawGroup = inData + pos;

                    pos += groupSize;

                    for(int i = 0; i < groupSize && decompPos < *outLength; i++) {

                        if(rawGroup + i >= inData + inLength) {
                            // Ran past the end of the buffer! Oh noes!
                            break;
                        }

                        decompressed[decompPos++] = rawGroup[i];
                    }

                    realLength--;

                    if(pos >= inLength) break;
                }
            }
        }

        if(pos >= inLength && decompPos >= *outLength) {

            // Everything went okay. Made it to the end of both buffers.

        } else {

            // Compression problems? Uncomment these...

            // if(pos < inLength) {
            // 	cout << "Didn't make it to the end of decompress input " << pos << " " << inLength << endl;
            // }
            // if(decompPos < *outLength) {
            // 	cout << "Didn't make it to the end of decompress output " << decompPos << " " << *outLength << endl;
            // }

            // Something went wrong.
            delete[] decompressed;
            decompressed = NULL;
            *outLength = 0;
        }

        return decompressed;

    }
}


