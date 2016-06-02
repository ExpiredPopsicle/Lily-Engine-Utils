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
#include <cstring>
using namespace std;

#include <lilyengine/utils.h>
#include <lilyengine/image.h>
#include <lilyengine/filesystem.h>
using namespace ExPop::Console;
using namespace ExPop;

// FIXME: Make this a function.
#define IS_POW2(x) (!((x) & ((x) - 1)))

namespace ExPop {

    namespace Gfx {

        Image::Image(void) {
            pixels = NULL;
            width = 0;
            height = 0;
        }

        Image::Image(int width, int height) {
            this->width = width;
            this->height = height;
            this->pixels = new Pixel[width * height];
        }

        Image::~Image(void) {
            delete[] pixels;
        }

        // Move forward in the input stream.
      #define INCPTR(x) if(p - (unsigned char*)tgaData + x > length) { if(img) delete img; return NULL; } p += x

        // Move forward in our output stream.  There's no overflow check here
        // because we allocate enough to begin with.  Uh.... in theory.
      #define SKIPBYTES(x) { for(int i = x; i > 0; i--) { *p = 0; p++; } }

        unsigned char *Image::saveTGA(int *length) const {

            // This assumes we want a 32-bit, uncompressed, non-color-mapped
            // TGA with alpha.

            *length = sizeof(Pixel) * width * height + 18;
            unsigned char *out = new unsigned char[*length];
            unsigned char *p = out;

            // We're going to skip through most of the header, except for the
            // imageType, which must be 2 (RGBA, uncompressed).

            SKIPBYTES(2); // idLength
            // colorMapType

            *p = 2;	p++;  // imageType

            SKIPBYTES(9); // firstEntryIndex (2)
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


        void Image::sampleNearest(
            float x, float y,
            float &r, float &g,
            float &b, float &a) const {

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


        void Image::sampleInterpolated(
            float x, float y,
            float &r, float &g,
            float &b, float &a) const {

            x *= float(getWidth());
            y *= float(getHeight());

            int x0 = int(x);
            float xs = x - float(int(x));

            int y0 = int(y);
            float ys = y - float(int(y));

            x0 = x0 % getWidth();
            y0 = y0 % getHeight();

            r = g = b = a = 0;

            assert(IS_POW2(getWidth()) && IS_POW2(getHeight()));

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

        void loadTGAHeaderGetSize(void *tgaData, int *width, int *height) {

            unsigned char *p = ((unsigned char*)tgaData) + 12;
            *width           = *(unsigned short*)p; p += 2;
            *height          = *(unsigned short*)p;

        }

        Image *loadTGA(void *tgaData, int length, bool convertPink) {

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
            INCPTR(idLength);

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
                    INCPTR(1);

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

                            INCPTR(4);
                            dp.rgba.r = *(p - 2);
                            dp.rgba.g = *(p - 3);
                            dp.rgba.b = *(p - 4);
                            dp.rgba.a = *(p - 1);

                        } else if(depth == 24) {

                            // 24 bit BGR.

                            INCPTR(3);
                            dp.rgba.r = *(p - 1);
                            dp.rgba.g = *(p - 2);
                            dp.rgba.b = *(p - 3);
                            dp.rgba.a = 255;

                        } else {

                            // Uh...... 16 or 15 bit?
                            dp.value = 0xffffffff;
                        }

                    } else if(imageType == 3 || imageType == 11) {

                        // Greyscale

                        if(depth == 16) {

                            // Grey with alpha.
                            INCPTR(2);
                            dp.rgba.b = dp.rgba.g = dp.rgba.r = *(p - 2);
                            dp.rgba.a = *(p - 1);

                        } else if(depth == 8) {

                            // Grey, no alpha.
                            INCPTR(1);
                            dp.rgba.b = dp.rgba.g = dp.rgba.r = *(p - 1);
                            dp.rgba.a = 255;

                        } else {

                            // I don't know blah blah blah
                            dp.value = 0xffffffff;

                        }

                    } else {

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

        Image *loadTGAFromFile(const std::string &filename) {
            char *fileData = NULL;
            int fileLen = 0;
            fileData = FileSystem::loadFile(filename, &fileLen);
            if(fileData) {
                Image *img = loadTGA(fileData, fileLen);
                delete[] fileData;
                return img;
            }
            return NULL;
        }

        Image *load1BitImageFromBitmap(
            char *bitmap,
            unsigned int length,
            unsigned int width,
            unsigned int height,
            bool alphaOnly) {

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

        Image *Image::makeHalfRes(void) {

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

                        Pixel *p = getPixel(x*2 + xo, y*2 + yo);

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

        bool Image::isPow2Size(void) {
            return !(width & (width - 1)) && !(height & (height - 1)) && height && width;
        }

    }

}


