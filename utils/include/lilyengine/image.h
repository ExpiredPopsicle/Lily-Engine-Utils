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

#pragma once

#include <cstdlib>
#include <string>

#ifdef WIN32
// Apparently Windows lacks stdint.h
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

namespace ExPop {

    namespace Gfx {

        /// A union representing a 32 Bit RGBA pixel.
        union Pixel {
            uint32_t value;
            struct {
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;
            } rgba;
            unsigned char colorsAsArray[4];
        };

        /// Integer rectangle.
        class Rect {
        public:

            /// Create a Rect between the specified points.
            inline Rect(int x1, int y1, int x2, int y2) {
                this->x1 = x1;
                this->y1 = y1;
                this->x2 = x2;
                this->y2 = y2;
            }

            /// Get the width of the rectangle.
            inline int getWidth(void) const {
                return std::abs(x2 - x1);
            }

            /// Get the height of the rectangle.
            inline int getHeight(void) const {
                return std::abs(y2 - y1);
            }

            int x1, y1;
            int x2, y2;

        private:
        };

        /// A basic 32 bit (RGBA) image buffer.
        class Image {
        public:

            /// Create a 0x0 image.
            Image(void);

            /// Create a new image with specified width and height.
            Image(int width, int height);

            /// Destructor.
            ~Image(void);

            /// Set a pixel.
            inline void setPixel(const Pixel &p, int x, int y) {
                setPixel(p, x + y * width);
            }

            /// Set a pixel based on linear index.
            inline void setPixel(const Pixel &p, int index) {
                if(index < 0 || index > width * height) return;
                pixels[index].value = p.value;
            }

            /// Get a pointer to a pixel.
            inline Pixel *getPixel(int x, int y) {
                if(x >= width || x < 0 || y >= height || y < 0) return NULL;
                return getPixel(x + y * width);
            }

            /// Get a pointer to a pixel based on linear index.
            inline Pixel *getPixel(int index) {
                if(index < 0 || index > width * height) return NULL;
                return &(pixels[index]);
            }

            /// Get a bilinear interpolated color value from some
            /// fractional point.
            void sampleInterpolated(
                float x, float y,
                float &r, float &g,
                float &b, float &a) const;

            /// Get the nearest pixel on the image to some texture
            /// coordinate.
            void sampleNearest(
                float x, float y,
                float &r, float &g,
                float &b, float &a) const;

            /// Get the width.
            inline int getWidth(void) const {
                return width;
            }

            /// Get the height.
            inline int getHeight(void) const {
                return height;
            }

            /// Save as a TGA to a new buffer. Only output format is
            /// 32-bit RGBA, uncompressed.
            unsigned char *saveTGA(int *length) const;

            /// Decompress a DXT image. This image must already be the
            /// right size.
            void decompressDXT(unsigned char *buffer, unsigned int bufferLength, unsigned int dxtLevel);

            /// Make an image that's half the resolution on each axis
            /// and return that. Use for generating mip levels and
            /// stuff.
            Image *makeHalfRes(void);

            /// Determine if the image is a 2^n size.
            bool isPow2Size(void);

        private:
            Pixel *pixels;
            int width;
            int height;
        };

        /// Load an image from a TGA file in a buffer.
        Image *loadTGA(void *tgaData, int length, bool convertPink = false);

        /// Load a TGA image from a file. Just wraps up all the
        /// filesystem junk and calls loadTGA().
        Image *loadTGAFromFile(const std::string &filename);

        /// Load an image from a 1-bit-per-pixel buffer.
        Image *load1BitImageFromBitmap(
            char *bitmap,
            unsigned int length,
            unsigned int width,
            unsigned int height,
            bool alphaOnly = false);

        /// Get the size of a TGA from just the header data. (Should
        /// be 16 bytes at least.)
        void loadTGAHeaderGetSize(void *tgaData, int *width, int *height);
    }
}

