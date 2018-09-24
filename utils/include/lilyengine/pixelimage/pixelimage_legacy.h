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

// Conversions to/from old ExPop::Gfx::Image stuff.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "pixelvalue.h"
#include "pixelimagebase.h"
#include "pixelimage.h"

#include "../image.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Make a new PixelImage from an old ExPop::Gfx::Image. Format will
    /// still be 32-bit RGBA.
    PixelImage<uint8_t> *pixelImageFromOldImage(ExPop::Gfx::Image &img);

    /// Convert an image to the old system. All channels after the first
    /// four are ignored. If there are under four, then more channels will
    /// be created to fill in the space, with the fourth being initialized
    /// to fully opaque alpha.
    ExPop::Gfx::Image *pixelImageToOldImage(PixelImageBase &img);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline PixelImage<uint8_t> *pixelImageFromOldImage(ExPop::Gfx::Image &img)
    {
        PixelImage<uint8_t> *newImage = new PixelImage<uint8_t>(
            img.getWidth(), img.getHeight(), 4);

        for(PixelImage_Coordinate y = 0; y < newImage->getHeight(); y++) {
            for(PixelImage_Coordinate x = 0; x < newImage->getWidth(); x++) {
                ExPop::Gfx::Pixel *p = img.getPixel(x, y);
                for(PixelImage_Coordinate c = 0; c < newImage->getChannelCount(); c++) {
                    newImage->getData(x, y, c).value = p->colorsAsArray[c];
                }
            }
        }

        return newImage;
    }

    inline ExPop::Gfx::Image *pixelImageToOldImage(PixelImageBase &img)
    {
        // Convert to uint8_t.
        PixelImage<uint8_t> newImage(img);

        // Bring it up to the 4-channel RGBA the old system uses.
        if(newImage.getChannelCount() < 4) {
            newImage.setNumChannels(4);

            // Fill in alpha to 1.
            for(PixelImage_Coordinate y = 0; y < newImage.getHeight(); y++) {
                for(PixelImage_Coordinate x = 0; x < newImage.getWidth(); x++) {
                    newImage.getData(x, y, 3).value = 0xff;
                }
            }
        }

        ExPop::Gfx::Image *ret = new ExPop::Gfx::Image(
            newImage.getWidth(), newImage.getHeight());

        for(PixelImage_Coordinate y = 0; y < newImage.getHeight(); y++) {
            for(PixelImage_Coordinate x = 0; x < newImage.getWidth(); x++) {
                ExPop::Gfx::Pixel *p = ret->getPixel(x, y);
                for(PixelImage_Coordinate c = 0; c < 4; c++) {
                    p->colorsAsArray[c] = newImage.getData(x, y, c).value;
                }
            }
        }

        return ret;
    }
}

