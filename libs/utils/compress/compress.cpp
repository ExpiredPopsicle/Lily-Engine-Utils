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

#include <cassert>
#include <cstring>
#include <iostream>
using namespace std;

#include "compress.h"

#define LENGTHBITS 15
#define MAXCHUNKLENGTH (1 << (LENGTHBITS - 1))

namespace ExPop {

    // This goes at the beginning of the whole compressed block of data.
    struct CompressHeader {
        unsigned int fullLength;
    };

    // This precedes every chunk of bytes, which may be a variable number
    // of raw or run length bytes.
    struct ChunkHeader {

        // 0 = raw length.
        // 1 = run length.
        unsigned short rle    : 1;

        // Length of the chunk. (1-128, so add one to this.)
        unsigned short length : LENGTHBITS;
    };

    static bool shouldDoRunLength(const char *inData, unsigned int lengthLeft, int groupSize) {

        unsigned int checkLen = sizeof(ChunkHeader) + (groupSize * 2);

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

    static bool nextFewBytesSame(const char *b1, const char *b2, int length, int lengthLeft) {

        if(length > lengthLeft) return false;

        for(int i = 0; i < length; i++) {

            if(b1[i] != b2[i]) {
                return false;
            }

        }

        return true;

    }

    char *compressRLE(const char *inData, unsigned int inLength, unsigned int *outLength, int groupSize) {

        // Worst case we end up with nothing but raw lengths. The max
        // length of that will then be the incoming length plus one chunk
        // header for every 128 bytes. (Plus one because C++ will round
        // down.) Also add the CompressHeader.
        unsigned int scratchAllocatedLength =
            (sizeof(CompressHeader) +
             inLength +
             ((groupSize * inLength / (MAXCHUNKLENGTH-1)) + 1) * sizeof(ChunkHeader));

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

            bool doRLE = shouldDoRunLength(inData + pos, inLength - pos, groupSize);

            if(doRLE) {

                char *runGroup = new char[groupSize];

                assert(scratchActualLength < scratchAllocatedLength);

                ChunkHeader *header = (ChunkHeader*)&(scratchBuffer[scratchActualLength]);
                scratchActualLength += sizeof(ChunkHeader);

                header->rle = 1;
                header->length = 0; // Zero actually means 1 here. For the first byte we got at runByte.

                // Record the bytes.
                for(int i = 0; i < groupSize; i++) {
                    runGroup[i] = inData[pos++];
                    scratchBuffer[scratchActualLength++] = runGroup[i];
                }

                //while(inData[pos] == runByte) {
                while(nextFewBytesSame(runGroup, inData + pos, groupSize, inLength - pos)) {

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

                ChunkHeader *header = (ChunkHeader*)&scratchBuffer[scratchActualLength];
                scratchActualLength += sizeof(ChunkHeader);

                header->rle = 0;
                header->length = 0; // This will be the unsigned equivalent of -1.

                while(!shouldDoRunLength(inData + pos, inLength - pos, groupSize)) {

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

    char *decompressRLE(const char *inData, unsigned int inLength, unsigned int *outLength, int groupSize) {

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
            ChunkHeader *header = (ChunkHeader*)&inData[pos];
            pos += sizeof(ChunkHeader);

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
};
