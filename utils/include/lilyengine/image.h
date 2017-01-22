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

// 32-bit RGBA Image class with TGA loader and saver. Save code
// doesn't do compression. Not all TGA formats supported.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "config.h"
#include "filesystem.h"

#include <cstdlib>
#include <string>
#include <cassert>
#include <cstring>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Gfx
    {
        /// A union representing a 32 Bit RGBA pixel, for the Image
        /// system.
        union Pixel
        {
            uint32_t value;
            struct {
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;
            } rgba;
            unsigned char colorsAsArray[4];
        };

        static_assert(sizeof(Pixel) == 4, "Pixel structure size is bad.");

        /// A basic 32 bit (RGBA) image buffer.
        class Image
        {
        public:

            /// Create a 0x0 image.
            Image(void);

            /// Create a new image with specified width and height.
            Image(int width, int height);

            /// Copy constructor.
            Image(const Image &other);

            /// Destructor.
            ~Image(void);

            /// Copy.
            Image &operator=(const Image &other);

            /// Set a pixel.
            void setPixel(const Pixel &p, int x, int y);

            /// Set a pixel based on linear index.
            void setPixel(const Pixel &p, unsigned int index);

            /// Get a pointer to a pixel.
            Pixel *getPixel(int x, int y);

            /// Get a pointer to a pixel based on linear index.
            Pixel *getPixel(int index);

            /// Get a pointer to a pixel. Skip bounds checking.
            /// Dangerous if you don't carefully check your image
            /// bounds. Faster for large image operations.
            Pixel *getPixelFast(int x, int y);

            /// Get a pointer to a pixel based on linear index. Skip
            /// bounds checking. Dangerous if you don't carefully
            /// check your image bounds. Faster for large image
            /// operations.
            Pixel *getPixelFast(int index);

            /// Get a pointer to a pixel. Skip bounds checking.
            /// Dangerous if you don't carefully check your image
            /// bounds. Faster for large image operations.
            const Pixel *getPixelFast(int x, int y) const;

            /// Get a pointer to a pixel based on linear index. Skip
            /// bounds checking. Dangerous if you don't carefully
            /// check your image bounds. Faster for large image
            /// operations.
            const Pixel *getPixelFast(int index) const;

            /// Get a pointer to a pixel.
            const Pixel *getPixel(int x, int y) const;

            /// Get a pointer to a pixel based on linear index.
            const Pixel *getPixel(int index) const;

            // Set the image to entirely transparent black.
            void clear();

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
            unsigned int getWidth(void) const;

            /// Get the height.
            unsigned int getHeight(void) const;

            /// Save as a TGA to a new buffer. Only output format is
            /// 32-bit RGBA, uncompressed.
            unsigned char *saveTGA(int *length) const;

            /// Decompress a DXT image. This image must already be the
            /// right size.
            void decompressDXT(
                const unsigned char *buffer,
                unsigned int bufferLength,
                unsigned int dxtLevel);

            /// Make an image that's half the resolution on each axis
            /// and return that. Use for generating mip levels and
            /// stuff.
            Image *makeHalfRes(void) const;

            /// Determine if the image is a 2^n size.
            bool isPow2Size(void) const;

        private:
            Pixel *pixels;
            unsigned int width;
            unsigned int height;
        };

        /// Load an image from a TGA file in a buffer.
        Image *loadTGA(void *tgaData, int length, bool convertPink = false);

        /// Load a TGA image from a file. Just wraps up all the
        /// filesystem junk and calls loadTGA().
        Image *loadTGAFromFile(const std::string &filename);

        /// Save equivalent to loadTGAFromFile.
        bool saveTGAToFile(const Image *img, const std::string &filename);

        /// Load an image from a 1-bit-per-pixel buffer. Use with the
        /// imgbuffer tool for embedding images in code.
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

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Gfx
    {
        inline Image::Image(void)
        {
            pixels = nullptr;
            width = 0;
            height = 0;
        }

        inline Image::Image(int width, int height)
        {
            this->width = width;
            this->height = height;
            this->pixels = new Pixel[width * height];
        }

        inline Image::Image(const Image &other)
        {
            this->width = 0;
            this->height = 0;
            this->pixels = nullptr;

            operator=(other);
        }

        inline Image &Image::operator=(const Image &other)
        {
            delete[] pixels;
            pixels = nullptr;

            this->width = other.width;
            this->height = other.height;

            if(other.pixels) {

                this->pixels = new Pixel[width * height];
                memcpy(
                    this->pixels,
                    other.pixels,
                    sizeof(Pixel) * width * height);

            } else {

                pixels = nullptr;

            }

            return *this;
        }

        inline Image::~Image(void)
        {
            delete[] pixels;
        }

        inline void Image::setPixel(const Pixel &p, int x, int y)
        {
            setPixel(p, x + y * width);
        }

        inline void Image::setPixel(const Pixel &p, unsigned int index)
        {
            if(index > width * height) return;
            pixels[index].value = p.value;
        }

        inline Pixel *Image::getPixel(int x, int y)
        {
            if(x >= int(width) || x < 0 || y >= int(height) || y < 0) return NULL;
            return getPixel(x + y * width);
        }

        inline Pixel *Image::getPixelFast(int x, int y)
        {
            return getPixelFast(x + y * width);
        }

        inline const Pixel *Image::getPixelFast(int x, int y) const
        {
            return getPixelFast(x + y * width);
        }

        inline Pixel *Image::getPixel(int index)
        {
            if(index < 0 || index > int(width * height)) return NULL;
            return &(pixels[index]);
        }

        inline Pixel *Image::getPixelFast(int index)
        {
            return &(pixels[index]);
        }

        inline const Pixel *Image::getPixelFast(int index) const
        {
            return &(pixels[index]);
        }

        inline const Pixel *Image::getPixel(int x, int y) const
        {
            if(x >= int(width) || x < 0 || y >= int(height) || y < 0) return NULL;
            return getPixel(x + y * width);
        }

        inline const Pixel *Image::getPixel(int index) const
        {
            if(index < 0 || index > int(width * height)) return NULL;
            return &(pixels[index]);
        }

        inline unsigned int Image::getWidth(void) const
        {
            return width;
        }

        inline unsigned int Image::getHeight(void) const
        {
            return height;
        }

        inline void Image::clear()
        {
            memset(pixels, 0, sizeof(Pixel) * width * height);
        }

        inline bool Image::isPow2Size(void) const
        {
            return !(width & (width - 1)) && !(height & (height - 1)) && height && width;
        }

        inline void Image::sampleNearest(
            float x, float y,
            float &r, float &g,
            float &b, float &a) const
        {
            x *= float(width);
            y *= float(height);

            int rx = int(x + 0.5) % width;
            int ry = int(y + 0.5) % height;

            Pixel *p = &pixels[rx + ry * width];

            assert(p);

            r = float(p->rgba.r) / 255.0f;
            g = float(p->rgba.g) / 255.0f;
            b = float(p->rgba.b) / 255.0f;
            a = float(p->rgba.a) / 255.0f;
        }

        inline void Image::sampleInterpolated(
            float x, float y,
            float &r, float &g,
            float &b, float &a) const
        {
            x *= float(getWidth());
            y *= float(getHeight());

            int x0 = int(x);
            float xs = x - float(int(x));

            int y0 = int(y);
            float ys = y - float(int(y));

            x0 = x0 % getWidth();
            y0 = y0 % getHeight();

            r = g = b = a = 0;

            assert(isPow2Size());

            for(unsigned int i = 0; i < 4; i++) {

                bool xd = !!(i & 1);
                bool yd = !!(i & 2);

                int rx = (x0 + xd) & (getWidth() - 1);
                int ry = (y0 + yd) & (getHeight() - 1);

                Pixel *p = &pixels[rx + ry * width];

                float s =
                    (xd ? xs : (1.0 - xs)) *
                    (yd ? ys : (1.0 - ys)) / 255.0;

                r += float(p->rgba.r) * s;
                g += float(p->rgba.g) * s;
                b += float(p->rgba.b) * s;
                a += float(p->rgba.a) * s;
            }

        }

        inline Image *Image::makeHalfRes(void) const
        {
            unsigned int newWidth = getWidth() / 2;
            unsigned int newHeight = getHeight() / 2;

            if(!newWidth) newWidth = 1;
            if(!newHeight) newHeight = 1;

            Image *newImg = new Image(newWidth, newHeight);

            for(unsigned int x = 0; x < newWidth; x++) {

                for(unsigned int y = 0; y < newHeight; y++) {

                    float color[4] = {0};

                    for(unsigned int i = 0; i < 4; i++) {

                        int xo = i % 2;
                        int yo = i / 2;

                        if(getWidth() == 1) {
                            xo = 0;
                        }

                        if(getHeight() == 1) {
                            yo = 0;
                        }

                        const Pixel *p = getPixel(x*2 + xo, y*2 + yo);

                        color[0] += p->rgba.r;
                        color[1] += p->rgba.g;
                        color[2] += p->rgba.b;
                        color[3] += p->rgba.a;

                    }

                    Pixel *outPixel = newImg->getPixel(x, y);
                    outPixel->rgba.r = int(color[0] / 4.0);
                    outPixel->rgba.g = int(color[1] / 4.0);
                    outPixel->rgba.b = int(color[2] / 4.0);
                    outPixel->rgba.a = int(color[3] / 4.0);
                }
            }

            return newImg;
        }

        // Move forward in our output stream.  There's no overflow check here
        // because we allocate enough to begin with.  Uh.... in theory.
      #define EXPOP_SAVETGA_SKIPBYTES(x) { for(int i = x; i > 0; i--) { *p = 0; p++; } }

        inline unsigned char *Image::saveTGA(int *length) const
        {
            // This assumes we want a 32-bit, uncompressed, non-color-mapped
            // TGA with alpha.

            *length = sizeof(Pixel) * width * height + 18;
            unsigned char *out = new unsigned char[*length];
            unsigned char *p = out;

            // We're going to skip through most of the header, except for the
            // imageType, which must be 2 (RGBA, uncompressed).

            EXPOP_SAVETGA_SKIPBYTES(2);
            // idLength
            // colorMapType

            *p = 2;	p++;  // imageType

            EXPOP_SAVETGA_SKIPBYTES(9);
            // firstEntryIndex (2)
            // colorMapLength (2)
            // colorMapEntrySize
            // xOrigin (2)
            // yOrigin (2)

            // Important stuff!

            // FIXME: Endian issues will happen here with
            // all the unsigned shorts.

            // Width and height
            *(unsigned short*)p = width;  p += 2;
            *(unsigned short*)p = height; p += 2;

            // Depth
            *p = 32; p++;

            // Flip bit thing. We need this or the image pops out upside down.
            *p = 1 << 5; p++;

            // Write out the pixels.
            for(unsigned int y = 0; y < height; y++) {
                for(unsigned int x = 0; x < width; x++) {
                    Pixel *dp = &(pixels[x + y * width]);

                    // TGA stores stuff in BGRA.
                    *p = dp->rgba.b; p++;
                    *p = dp->rgba.g; p++;
                    *p = dp->rgba.r; p++;
                    *p = dp->rgba.a; p++;
                }
            }

            return out;
        }

        // ----------------------------------------------------------------------
        // TGA-related stuff follows.
        // ----------------------------------------------------------------------

        // Move forward in the input stream.
      #define EXPOP_LOADTGA_INCPTR(x) if(size_t(p - (unsigned char*)tgaData + x) > size_t(length)) { if(img) delete img; return NULL; } p += x

        inline void loadTGAHeaderGetSize(void *tgaData, int *width, int *height)
        {
            unsigned char *p = ((unsigned char*)tgaData) + 12;
            *width           = *(unsigned short*)p; p += 2;
            *height          = *(unsigned short*)p;
        }

        inline Image *loadTGA(void *tgaData, int length, bool convertPink)
        {
            Image *img = NULL;

            int idLength;
            // int colorMapType;
            int imageType;

            // Color map crap.
            // int firstEntryIndex;
            int colorMapLength;
            int colorMapEntrySize;

            // This is meaningless to me~
            // int xOrigin;
            // int yOrigin;

            int width;
            int height;
            int depth;
            int imageDescriptor;

            unsigned char *p = (unsigned char*)tgaData;

            // Buffer not even long enough for the header?
            if(length < 18) return NULL;

            // Read in the header.
            // FIXME: Endian issues will be here.
            idLength          = *p; p++;

            // colorMapType      = *p;
            p++;

            imageType         = *p; p++;

            // firstEntryIndex   = *(unsigned short*)p;
            p += 2;

            colorMapLength    = *(unsigned short*)p; p += 2;
            colorMapEntrySize = *p; p++;

            // xOrigin           = *(unsigned short*)p;
            p += 2;

            // yOrigin           = *(unsigned short*)p;
            p += 2;

            width             = *(unsigned short*)p; p += 2;
            height            = *(unsigned short*)p; p += 2;
            depth             = *p; p++;
            imageDescriptor   = *p; p++;

            // Skip image descriptor.
            EXPOP_LOADTGA_INCPTR(idLength);

            // This loader only works with true-color images and greyscale
            // right now. RLE and non-RLE are okay, though.
            switch(imageType) {
                case 2:  // Truecolor, uncompressed.
                case 3:  // Greyscale, uncompressed.
                case 10: // Truecolor, RLE
                case 11: // Greyscale, RLE
                    break;
                default:
                    // Can't load this image. Not the right type.
                    return NULL;
            }

            // TODO: Add colormap support?

            int colorMapSize;
            if(colorMapEntrySize != 15) {
                colorMapSize = (colorMapLength * colorMapEntrySize) / 8;
            } else {
                // If the color map entries are 15 bits each, we still have to
                // read them as though they're 16 bits, and just ignore the
                // last bit.
                colorMapSize = (colorMapLength * 16) / 8;
            }

            // Skip color map. Lalala
            if(length < 18 + colorMapSize) return NULL;
            p += colorMapSize;

            // Allocate the image.
            img = new Image(width, height);
            if(!img) return NULL;

            int totalPx = width * height;
            int index = 0;

            bool useRle = (imageType >= 9 && imageType <= 11);
            bool flipVertically = !(imageDescriptor & (1 << 5));

            if(flipVertically) {
                // If we're vertically flipped, then we need to start on the last row.
                index = width * (height - 1);
            }

            while(index < totalPx && index >= 0) {

                int runLength = 1;
                int rawLength = 1;

                if(useRle) {
                    // Get the number of pixels this next thing represents.
                    runLength = (*p);
                    EXPOP_LOADTGA_INCPTR(1);

                    // Figure out if it was actually a run length or a raw
                    // length. Note that the value read in is actually the
                    // number of bytes minus one, so we need to add one to
                    // whichever one of these it was.
                    if(runLength >= 128) {
                        // Run length. Remove the first bit and add
                        // 1. (Subtracting 127 does this.)
                        runLength -= 127;
                        rawLength = 1;
                    } else {
                        // Raw length. Add one and swap run/raw.
                        rawLength = runLength + 1;
                        runLength = 1;
                    }
                }

                while(rawLength) {

                    // Get the pixel value.
                    Pixel dp;

                    if(imageType == 10 || imageType == 2) {

                        // True color

                        if(depth == 32) {

                            // 32 bit BGRA.

                            EXPOP_LOADTGA_INCPTR(4);
                            dp.rgba.r = *(p - 2);
                            dp.rgba.g = *(p - 3);
                            dp.rgba.b = *(p - 4);
                            dp.rgba.a = *(p - 1);

                        } else if(depth == 24) {

                            // 24 bit BGR.

                            EXPOP_LOADTGA_INCPTR(3);
                            dp.rgba.r = *(p - 1);
                            dp.rgba.g = *(p - 2);
                            dp.rgba.b = *(p - 3);
                            dp.rgba.a = 255;

                        } else {

                            // FIXME: 16 or 15 bits unsupported. Fail
                            // more elegantly.

                            // Uh...... 16 or 15 bit?
                            dp.value = 0xffffffff;
                        }

                    } else if(imageType == 3 || imageType == 11) {

                        // Greyscale

                        if(depth == 16) {

                            // Grey with alpha.
                            EXPOP_LOADTGA_INCPTR(2);
                            dp.rgba.b = dp.rgba.g = dp.rgba.r = *(p - 2);
                            dp.rgba.a = *(p - 1);

                        } else if(depth == 8) {

                            // Grey, no alpha.
                            EXPOP_LOADTGA_INCPTR(1);
                            dp.rgba.b = dp.rgba.g = dp.rgba.r = *(p - 1);
                            dp.rgba.a = 255;

                        } else {

                            // I don't know blah blah blah
                            dp.value = 0xffffffff;

                        }

                    } else {

                        // FIXME: This image format is unsupported.
                        // Maybe we should fail more elegantly.

                        // I don't even know what this is blah blah blah.
                        dp.value = 0xffffffff;

                    }

                    if(convertPink) {

                        // Convert 0xff00ff?? to alpha 0.
                        if(dp.rgba.r == 255 && dp.rgba.g == 0 && dp.rgba.b == 255) {
                            dp.rgba.a = 0;
                        }
                    }

                    // Set the pixels.
                    while(runLength) {

                        img->setPixel(dp, index);

                        index++;

                        if(flipVertically) {
                            if(!(index % width)) {
                                // Went past an edge.
                                index -= width * 2;
                            }
                        }

                        runLength--;
                    }

                    runLength = 1;
                    rawLength--;
                }
            }

            return img;
        }

        inline Image *loadTGAFromFile(const std::string &filename)
        {
            char *fileData = NULL;
            int64_t fileLen = 0;
            fileData = FileSystem::loadFile(filename, &fileLen);
            if(fileData) {
                Image *img = loadTGA(fileData, fileLen);
                delete[] fileData;
                return img;
            }
            return NULL;
        }

        inline bool saveTGAToFile(const Image *img, const std::string &filename)
        {
            int imgLen = 0;
            unsigned char *data = img->saveTGA(&imgLen);

            bool ret = (0 == FileSystem::saveFile(filename, (char*)data, imgLen));

            delete[] data;

            return ret;
        }

        // ----------------------------------------------------------------------
        // imgbuffer stuff follows
        // ----------------------------------------------------------------------

        inline Image *load1BitImageFromBitmap(
            char *bitmap,
            unsigned int length,
            unsigned int width,
            unsigned int height,
            bool alphaOnly)
        {
            Image *img = new Image(width, height);
            if(!img) return NULL;

            if(width * height != length * 8) {
                return NULL;
            }

            unsigned int pixelIndex = 0;

            for(unsigned int byteIndex = 0; byteIndex < length; byteIndex++) {
                for(unsigned int bitIndex = 0; bitIndex < 8; bitIndex++) {

                    Pixel *px = img->getPixel(pixelIndex);
                    pixelIndex++;

                    bool white = bitmap[byteIndex] & (1 << (7-bitIndex));

                    // FIXME: Little bit lazy about endianness here.
                    if(white) {
                        px->value = 0xffffffff;
                    } else {
                        if(alphaOnly) {
                            px->value = 0xffffffff;
                            px->rgba.a = 0x00;
                        } else {
                            px->value = 0x00000000;
                            px->rgba.a = 0xff;
                        }
                    }
                }
            }

            return img;
        }

        // ----------------------------------------------------------------------
        // DXT stuff follows
        // ----------------------------------------------------------------------

        // Note: It's been forever since the DXT code here was tested.
        // It has probably suffered from code rot and could probably
        // use a cleanup pass anyway.

        // Integer-only alpha blend.
        inline Pixel interpColor_8bitAlpha(
            const Pixel *a, const Pixel *b, unsigned int alpha)
        {
            Pixel ret;
            unsigned int minusAlpha = 255 - alpha;
            ret.rgba.r = (int(a->rgba.r) * minusAlpha + int(b->rgba.r) * alpha) >> 8;
            ret.rgba.g = (int(a->rgba.g) * minusAlpha + int(b->rgba.g) * alpha) >> 8;
            ret.rgba.b = (int(a->rgba.b) * minusAlpha + int(b->rgba.b) * alpha) >> 8;
            ret.rgba.a = (int(a->rgba.a) * minusAlpha + int(b->rgba.a) * alpha) >> 8;

            return ret;
        }

        const uint32_t MASK_RED_16BIT   = 0xF800; // 1111 1000 0000 0000
        const uint32_t MASK_GREEN_16BIT = 0x07E0; // 0000 0111 1110 0000
        const uint32_t MASK_BLUE_16BIT  = 0x001F; // 0000 0000 0001 1111

        inline Pixel color16BitTo32Bit(
            uint16_t color)
        {
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

        inline unsigned int getBitInBlock(
            const unsigned char *block, unsigned int bitIndex)
        {
            block += (bitIndex / 8);
            bitIndex %= 8;
            return !!((*block) & (0x1 << bitIndex));
        }

        inline unsigned int someBitsFromBlock(
            const unsigned char *block,
            unsigned int index,
            unsigned int howMany)
        {
            unsigned char ret = 0;
            for(unsigned int i = 0; i < howMany; i++) {
                ret += getBitInBlock(block, index * howMany + i) << i;
            }
            return ret;
        }

        // TODO: Make this take const input.
        // Input is 8 bytes. (2 * 16 bits colors + 4*4*2 bits data)
        // Output is 512 bytes. (32-bit colors * 4x4 pixels)
        inline void decompressDXTBlock(
            const unsigned char *block,
            unsigned char *outBlock,
            unsigned int dxtLevel)
        {
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

            const unsigned char *dataStart = block + 4;
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

      #if EXPOP_ENABLE_SQUISH

        unsigned char *Image::compressDXT(unsigned int *bufferLength, unsigned int *dxtLevel)
        {
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

        inline void Image::decompressDXT(
            const unsigned char *buffer,
            unsigned int bufferLength,
            unsigned int dxtLevel)
        {
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

            const unsigned char* inPtr = buffer;

            for(unsigned int y = 0; y < getHeight(); y += 4) {
                for(unsigned int x = 0; x < getWidth(); x += 4) {

                    unsigned char decompressedPixels[16*4];

                    decompressDXTBlock(
                        inPtr, (unsigned char*)decompressedPixels,
                        dxtLevel);

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


