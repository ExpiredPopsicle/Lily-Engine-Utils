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

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>

using namespace std;

#include <utils.h>
using namespace ExPop::Console;
using namespace ExPop;

#include "image.h"

namespace ExPop {

    namespace Gfx {

        // Integer-only alpha blend.
        static inline Pixel interpColor_8bitAlpha(
            const Pixel *a, const Pixel *b, unsigned int alpha) {

            Pixel ret;
            unsigned int minusAlpha = 255 - alpha;
            ret.rgba.r = (int(a->rgba.r) * minusAlpha + int(b->rgba.r) * alpha) >> 8;
            ret.rgba.g = (int(a->rgba.g) * minusAlpha + int(b->rgba.g) * alpha) >> 8;
            ret.rgba.b = (int(a->rgba.b) * minusAlpha + int(b->rgba.b) * alpha) >> 8;
            ret.rgba.a = (int(a->rgba.a) * minusAlpha + int(b->rgba.a) * alpha) >> 8;

            return ret;
        }

      #define MASK_RED_16BIT   0xF800 // 1111 1000 0000 0000
      #define MASK_GREEN_16BIT 0x07E0 // 0000 0111 1110 0000
      #define MASK_BLUE_16BIT  0x001F // 0000 0000 0001 1111

        static inline Pixel color16BitTo32Bit(
            uint16_t color) {

            // All I'm doing for each of these is masking it to get just
            // the bits for the channel, then shifting it so the MSB is at
            // bit 8.

            Pixel ret;
            ret.rgba.r = (unsigned int)((color & MASK_RED_16BIT)   >> 8);
            ret.rgba.g = (unsigned int)((color & MASK_GREEN_16BIT) >> 3);
            ret.rgba.b = (unsigned int)((color & MASK_BLUE_16BIT)  << 3);
            ret.rgba.a = 255;

            return ret;
        }

        static inline unsigned int getBitInBlock(
            unsigned char *block, unsigned int bitIndex) {
            block += (bitIndex / 8);
            bitIndex %= 8;
            return !!((*block) & (0x1 << bitIndex));
        }

        static inline unsigned int someBitsFromBlock(
            unsigned char *block,
            unsigned int index,
            unsigned int howMany) {

            unsigned char ret = 0;
            for(unsigned int i = 0; i < howMany; i++) {
                ret += getBitInBlock(block, index * howMany + i) << i;
            }
            return ret;
        }

        // TODO: Make this take const input.
        // Input is 8 bytes. (2 * 16 bits colors + 4*4*2 bits data)
        // Output is 512 bytes. (32-bit colors * 4x4 pixels)
        void decompressDXTBlock(unsigned char *block, unsigned char *outBlock, unsigned int dxtLevel) {

            // DXT3/5-specific stuff (or just setting up a table of fully
            // opaque alpha)
            // ----------------------------------------------------------------------

            unsigned char alphaValues[16];

            if(dxtLevel == 5) {

                // DXT5 is two representative alpha values, and then a
                // bunch of interpolations between those or pure
                // transparent or opaque.

                unsigned int alpha_0 = block[0];
                unsigned int alpha_1 = block[1];

                for(unsigned int i = 0; i < 16; i++) {

                    unsigned int sb = someBitsFromBlock(block+2, i, 3);

                    // This big tree of if/elses is just representing the
                    // decoding stuff from the spec directly.

                    if(sb == 0) {

                        alphaValues[i] = alpha_0;

                    } else if(sb == 1) {

                        alphaValues[i] = alpha_1;

                    } else {

                        if(alpha_0 > alpha_1) {

                            // First table of interpolation junk.
                            alphaValues[i] =
                                ((8-sb)*alpha_0 + (sb-1)*alpha_1) / 7;

                        } else {

                            if(sb == 6) {

                                alphaValues[i] = 0;

                            } else if(sb == 7) {

                                alphaValues[i] = 255;

                            } else {

                                // Second smaller table of interpolation
                                // junk.
                                alphaValues[i] =
                                    ((6-sb)*alpha_0 + (sb-1)*alpha_1) / 5;
                            }
                        }
                    }
                }

                // Now skip ahead so when the DXT1 part starts it'll just
                // be able to decode normally.
                block += 8;

            } else if(dxtLevel == 3) {

                // DXT3 alpha is just four bits per pixel.
                for(unsigned int i = 0; i < 16; i++) {
                    unsigned int sb = someBitsFromBlock(block, i, 4);
                    alphaValues[i] = sb * 17;
                }

            } else {

                // Just generate full alpha.
                for(unsigned int i = 0; i < 16; i++) {
                    alphaValues[i] = 0xff;
                }

            }

            // DXT1-specific stuff (handle all RGB)
            // ----------------------------------------------------------------------

            // Note that the DXT spec specifies that in DXT5, the opaque
            // RGB block must be handled as though the first color is
            // always greater than the second, but in DXT1 the decoding
            // depends on the order of the two colors.

            // Extract 16-bit versions of the two colors.
            uint16_t color16_0 = (unsigned int)(block[0]) + ((unsigned int)(block[1]) << 8);
            uint16_t color16_1 = (unsigned int)(block[2]) + ((unsigned int)(block[3]) << 8);

            // Convert those to RGBA.
            Pixel color32_0 = color16BitTo32Bit(color16_0);
            Pixel color32_1 = color16BitTo32Bit(color16_1);

            unsigned char *dataStart = block + 4;
            unsigned char *dataOut = outBlock;

            for(unsigned int i = 0; i < 16; i++) {
                unsigned int bits = someBitsFromBlock(dataStart, i, 2);

                Pixel finalColor;

                if(bits == 1) {

                    finalColor = color32_1;

                } else if(bits == 0) {

                    finalColor = color32_0;

                } else if(color16_0 > color16_1 ||
                          dxtLevel == 3 ||
                          dxtLevel == 5) { // DXT3 and DXT5 handled differently.

                    if(bits == 3) {
                        finalColor = interpColor_8bitAlpha(&color32_0, &color32_1, 85);
                    }

                    if(bits == 2) {
                        finalColor = interpColor_8bitAlpha(&color32_0, &color32_1, 171);
                    }

                } else {

                    finalColor.value = 0xffffffff;

                    if(bits == 3) {

                        finalColor.value = 0x00000000;

                        // This is DXT1's 1-bit alpha support.
                        alphaValues[i] = 0;
                    }

                    if(bits == 2) {
                        finalColor = interpColor_8bitAlpha(&color32_0, &color32_1, 128);
                    }

                }

                finalColor.rgba.a = alphaValues[i];

                *dataOut = finalColor.rgba.r; dataOut++;
                *dataOut = finalColor.rgba.g; dataOut++;
                *dataOut = finalColor.rgba.b; dataOut++;
                *dataOut = finalColor.rgba.a; dataOut++;
            }
        }

      #if EXPOP_USE_SQUISH

        unsigned char *Image::compressDXT(unsigned int *bufferLength, unsigned int *dxtLevel) {

            assert(isPow2Size());
            assert(getWidth() >= 4);
            assert(getHeight() >= 4);

            // First determine if this image completely opaque or not.
            bool completelyOpaque = true;
            for(int x = 0; x < getWidth() && completelyOpaque; x += 4) {
                for(int y = 0; y < getHeight() && completelyOpaque; y += 4) {
                    Pixel *p = getPixel(x, y);
                    if(p->rgba.a != 255) {
                        completelyOpaque = false;
                    }
                }
            }

            *dxtLevel = completelyOpaque ? 1 : 5;

            // Figure out buffer length based on whether we're using DXT1 or
            // DXT3/5 compression.
            *bufferLength =
                (getWidth() / 4) * (getHeight() / 4) * (completelyOpaque ? 8 : 16);

            unsigned char *outBuffer = new unsigned char[*bufferLength];

            unsigned char *bufPtr = outBuffer;

            for(int y = 0; y < getHeight(); y += 4) {
                for(int x = 0; x < getWidth(); x += 4) {

                    unsigned char inBuffer[16*4];

                    for(int i = 0; i < 16; i++) {

                        int xo = i % 4;
                        int yo = i / 4;

                        Pixel *p = getPixel(x + xo, y + yo);

                        inBuffer[i * 4] = p->rgba.r;
                        inBuffer[i * 4 + 1] = p->rgba.g;
                        inBuffer[i * 4 + 2] = p->rgba.b;
                        inBuffer[i * 4 + 3] = p->rgba.a;
                    }

                    squish::Compress(
                        (squish::u8*)inBuffer,
                        (squish::u8*)bufPtr,
                        completelyOpaque ? squish::kDxt1 : squish::kDxt5);

                    bufPtr += completelyOpaque ? 8 : 16;

                }

            }

            assert(outBuffer + *bufferLength == bufPtr);

            return outBuffer;

        }

      #endif

        void Image::decompressDXT(unsigned char *buffer, unsigned int bufferLength, unsigned int dxtLevel) {

            assert(dxtLevel == 1 || dxtLevel == 3 || dxtLevel == 5);
            assert(!(getWidth() % 4));
            assert(!(getHeight() % 4));

            // DXT formats with alpha have double the bits.
            unsigned int blockSize = 8;
            if(dxtLevel == 5 || dxtLevel == 3) blockSize = 16;

            // FIXME: Do something sensible if the buffer sizes don't
            // match up.
            unsigned int expectedNumBlocks = (getWidth() / 4) * (getHeight() / 4);
            unsigned int expectedBufferLength = blockSize * expectedNumBlocks;
            assert(expectedBufferLength == bufferLength);

            unsigned char* inPtr = buffer;

            for(unsigned int y = 0; y < getHeight(); y += 4) {
                for(unsigned int x = 0; x < getWidth(); x += 4) {

                    unsigned char decompressedPixels[16*4];

                    decompressDXTBlock(
                        (unsigned char*)inPtr, (unsigned char*)decompressedPixels, dxtLevel);

                    inPtr += blockSize;

                    for(int i = 0; i < 16; i++) {

                        int xo = i % 4;
                        int yo = i / 4;

                        Pixel *p = getPixel(x + xo, y + yo);

                        p->rgba.r = decompressedPixels[i * 4];
                        p->rgba.g = decompressedPixels[i * 4 + 1];
                        p->rgba.b = decompressedPixels[i * 4 + 2];
                        p->rgba.a = decompressedPixels[i * 4 + 3];
                    }
                }
            }
        }
    }
}
