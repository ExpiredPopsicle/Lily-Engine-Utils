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

// Blitting functions for PixelImage.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "pixelimagebase.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Blit from source to destination. Set alphaChannelIndex to -1
    /// to ignore alpha. wrapDst controls whether to wrap coordinates
    /// in the destination image (true) or just clip to the image
    /// edge. overrideAlpha comes into play only when
    /// alphaChannelIndex is -1, as the alpha value to use instead of
    /// the image's own alpha channel.
    void pixelImageBlit(
        const PixelImageBase &src,
        PixelImage_Coordinate src_left,
        PixelImage_Coordinate src_top,
        PixelImageBase &dst,
        PixelImage_Coordinate dst_left,
        PixelImage_Coordinate dst_top,
        PixelImage_Dimension width,
        PixelImage_Dimension height,
        PixelImage_Coordinate alphaChannelIndex = 3,
        double overrideAlpha = 1.0f,
        bool wrapDst = false);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline void pixelImageBlit(
        const PixelImageBase &src,
        PixelImage_Coordinate src_left,
        PixelImage_Coordinate src_top,
        PixelImageBase &dst,
        PixelImage_Coordinate dst_left,
        PixelImage_Coordinate dst_top,
        PixelImage_Dimension width,
        PixelImage_Dimension height,
        PixelImage_Coordinate alphaChannelIndex,
        double overrideAlpha,
        bool wrapDst)
    {

        for(PixelImage_Coordinate x = 0; x < width; x++) {
            for(PixelImage_Coordinate y = 0; y < height; y++) {
                for(PixelImage_Coordinate c = 0; c < dst.getChannelCount(); c++) {

                    // Preserve destination alpha channel.
                    if(c == alphaChannelIndex) {
                        continue;
                    }

                    PixelImage_Coordinate dstx = x + dst_left;
                    PixelImage_Coordinate dsty = y + dst_top;

                    if(wrapDst || (
                            (dstx >= 0 && dstx < dst.getWidth()) &&
                            (dsty >= 0 && dsty < dst.getHeight())))
                    {
                        double alpha = alphaChannelIndex == -1 ? overrideAlpha :
                            src.getDouble(x + src_left, y + src_top, alphaChannelIndex);

                        double srcVal =
                            src.getDouble(x + src_left, y + src_top, c);

                        double dstVal =
                            dst.getDouble(x + dst_left, y + dst_top, c);

                        dst.setDouble(dstx, dsty, c, dstVal * (1.0f - alpha) + srcVal * alpha);
                    }
                }
            }
        }


    }
}

