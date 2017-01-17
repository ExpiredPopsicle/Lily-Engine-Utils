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

// TGA support for arbitrary-format image class.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "pixelvalue.h"
#include "pixelimage.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    PixelImage<uint8_t> *pixelImageLoadTGA(const void *tgaData, size_t tgaDataLength);
    PixelImage<uint8_t> *pixelImageLoadTGAFromFile(const std::string &filename);
    uint8_t *pixelImageSaveTGA(const PixelImage<uint8_t> &img, size_t *length);
    bool pixelImageSaveTGAToFile(const PixelImage<uint8_t> &img, const std::string &filename);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{

    // Move forward in the input stream.
  #define EXPOP_LOADTGA2_INCPTR(x) if(p - (unsigned char*)tgaData + x > length) { if(img) delete img; return nullptr; } p += x

    inline PixelImage<uint8_t> *pixelImageLoadTGA(const void *tgaData, size_t length)
    {
        PixelImage<uint8_t> *img = NULL;

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
        if(length < 18) {
            return nullptr;
        }

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

        if(!width || !height || !depth) {
            return nullptr;
        }

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
        img = new PixelImage<uint8_t>(width, height, 4);

        size_t totalPx = width * height;
        size_t index = 0;

        bool useRle = (imageType >= 9 && imageType <= 11);
        bool flipVertically = !(imageDescriptor & (1 << 5));

        if(flipVertically) {
            // If we're vertically flipped, then we need to start on the last row.
            index = width * (height - 1);
        }

        while(index < totalPx && index != ~(size_t)0) {

            int runLength = 1;
            int rawLength = 1;

            if(useRle) {
                // Get the number of pixels this next thing represents.
                runLength = (*p);
                EXPOP_LOADTGA2_INCPTR(1);

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
                uint8_t dp[4];

                if(imageType == 10 || imageType == 2) {

                    // True color

                    if(depth == 32) {

                        // 32 bit BGRA.

                        EXPOP_LOADTGA2_INCPTR(4);
                        dp[0] = *(p - 2);
                        dp[1] = *(p - 3);
                        dp[2] = *(p - 4);
                        dp[3] = *(p - 1);

                    } else if(depth == 24) {

                        // 24 bit BGR.

                        EXPOP_LOADTGA2_INCPTR(3);
                        dp[0] = *(p - 1);
                        dp[1] = *(p - 2);
                        dp[2] = *(p - 3);

                        dp[3] = 255;

                    } else {

                        // FIXME: 16 or 15 bits unsupported. Fail
                        // more elegantly.

                        // Uh...... 16 or 15 bit?
                        dp[0] = 0xff;
                        dp[1] = 0xff;
                        dp[2] = 0xff;
                        dp[3] = 0xff;
                    }

                } else if(imageType == 3 || imageType == 11) {

                    // Greyscale

                    if(depth == 16) {

                        // Grey with alpha.
                        EXPOP_LOADTGA2_INCPTR(2);
                        dp[0] = dp[1] = dp[2] = *(p - 2);
                        dp[3] = *(p - 1);

                    } else if(depth == 8) {

                        // Grey, no alpha.
                        EXPOP_LOADTGA2_INCPTR(1);
                        dp[0] = dp[1] = dp[2] = *(p - 1);
                        dp[3] = 255;

                    } else {

                        // I don't know blah blah blah
                        dp[0] = 0xff;
                        dp[1] = 0xff;
                        dp[2] = 0xff;
                        dp[3] = 0xff;

                    }

                } else {

                    // FIXME: This image format is unsupported.
                    // Maybe we should fail more elegantly.

                    // I don't even know what this is blah blah blah.
                    dp[0] = 0xff;
                    dp[1] = 0xff;
                    dp[2] = 0xff;
                    dp[3] = 0xff;

                }

                // Set the pixels.
                while(runLength) {

                    for(size_t i = 0; i < 4; i++) {
                        img->getData(index % width, index / width, i).value = dp[i];
                    }

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

    inline PixelImage<uint8_t> *pixelImageLoadTGAFromFile(const std::string &filename)
    {
        char *fileData = nullptr;
        int64_t fileLen = 0;
        fileData = ExPop::FileSystem::loadFile(filename, &fileLen);
        if(fileData) {
            PixelImage<uint8_t> *img = pixelImageLoadTGA(fileData, fileLen);
            delete[] fileData;
            return img;
        }
        return nullptr;
    }

    // Move forward in our output stream.  There's no overflow check here
    // because we allocate enough to begin with.  Uh.... in theory.
  #define EXPOP_SAVETGA2_SKIPBYTES(x) { for(int i = x; i > 0; i--) { *p = 0; p++; } }

    inline uint8_t *pixelImageSaveTGA(
        const PixelImage<uint8_t> &img, size_t *length)
    {
        // This assumes we want a 32-bit, uncompressed, non-color-mapped
        // TGA with alpha.

        *length = 4 * img.getWidth() * img.getHeight() + 18;
        uint8_t *out = new uint8_t[*length];
        uint8_t *p = out;

        // We're going to skip through most of the header, except for the
        // imageType, which must be 2 (RGBA, uncompressed).

        EXPOP_SAVETGA2_SKIPBYTES(2);
        // idLength
        // colorMapType

        *p = 2;	p++;  // imageType

        EXPOP_SAVETGA2_SKIPBYTES(9);
        // firstEntryIndex (2)
        // colorMapLength (2)
        // colorMapEntrySize
        // xOrigin (2)
        // yOrigin (2)

        // Important stuff!

        // FIXME: Endian issues will happen here with
        // all the unsigned shorts.

        // Width and height
        *(unsigned short*)p = img.getWidth();  p += 2;
        *(unsigned short*)p = img.getHeight(); p += 2;

        // Depth
        *p = 32; p++;

        // Flip bit thing. We need this or the image pops out upside down.
        *p = 1 << 5; p++;

        // Write out the pixels.
        for(size_t y = 0; y < img.getHeight(); y++) {
            for(size_t x = 0; x < img.getWidth(); x++) {

                // TGA stores stuff in BGRA.

                *p = img.getData(x, y, 2).value; p++;
                *p = img.getData(x, y, 1).value; p++;
                *p = img.getData(x, y, 0).value; p++;
                *p = img.getData(x, y, 3).value; p++;

            }
        }

        return out;
    }

    inline bool pixelImageSaveTGAToFile(
        const PixelImage<uint8_t> &img, const std::string &filename)
    {
        size_t imgLen = 0;
        uint8_t *data = pixelImageSaveTGA(img, &imgLen);

        bool ret = (0 == ExPop::FileSystem::saveFile(filename, (char*)data, imgLen));

        delete[] data;

        return ret;
    }
}
