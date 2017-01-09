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

// Simple text rendering (ASCII only, fixed-width only) that renders
// to an ExPop::Gfx::Image.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "../matrix.h"
#include "../image.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Gfx
    {
        /// Render text onto an image.
        ///
        /// font input image must have width and height that are
        /// multiples of 16. The font input image represents the 256
        /// ASCII characters, including the code page 437 graphical
        /// characters, with 16 rows of 16 characters.
        ///
        /// If baseTex is used, then the font will be considered a
        /// mask to render baseTex for each letter. Make sure baseTex
        /// is the same size or larger than the character size in the
        /// font.
        void drawText(
            Image *font,
            Image *dst,
            Image *baseTex,
            int x, int y,
            const std::string &str,
            int lineSpacing = 0,
            int charSpacing = 0);

        /// Make all the black areas of an image transparent. This is
        /// basically just cheap color keying.
        void makeBlackTransparent(
            Image &img);

        /// Generate an outlined font for better reading on complex
        /// backgrounds. This will generate a bigger image to make
        /// room for the one-pixel outline.
        Image *generateOutlinedFontMask(
            Image *font);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Gfx
    {
        inline void makeBlackTransparent(
            ExPop::Gfx::Image &img)
        {
            for(size_t y = 0; y < img.getHeight(); y++) {
                for(size_t x = 0; x < img.getWidth(); x++) {
                    ExPop::Gfx::Pixel *outp = img.getPixelFast(x, y);
                    if(outp->rgba.r == 0) {
                        outp->rgba.a = 0;
                    }
                }
            }
        }

        class Rectangle
        {
        public:
            int x;
            int y;
            int w;
            int h;
        };

        // TODO: Maybe "expose" the blitting interface outside of the
        // font system.
        inline void imgBlit(
            const ExPop::Gfx::Image &src,
            const ExPop::Gfx::Rectangle &srcRect,
            ExPop::Gfx::Image &dst,
            const ExPop::Gfx::Rectangle &dstRect)
        {
            ExPop::Gfx::Rectangle realBounds = dstRect;

            // Check if output is before the buffer on either axis.
            if(realBounds.x + realBounds.w <= 0) {
                return;
            }
            if(realBounds.y + realBounds.h <= 0) {
                return;
            }

            // Check if output starts off the end on either axis.
            if(realBounds.x >= int(dst.getWidth())) {
                return;
            }
            if(realBounds.y >= int(dst.getHeight())) {
                return;
            }

            // Clamp area if we're over the edge.
            if(realBounds.x + realBounds.w > int(dst.getWidth())) {
                realBounds.w = dst.getWidth() - realBounds.x;
            }
            if(realBounds.y + realBounds.h > int(dst.getHeight())) {
                realBounds.h = dst.getHeight() - realBounds.y;
            }

            // Handle the case of starting off the edge.
            int xStartOffset = 0;
            if(dstRect.x < 0) {
                xStartOffset = -dstRect.x;
            }

            int yStartOffset = 0;
            if(dstRect.y < 0) {
                yStartOffset = -dstRect.y;
            }

            for(int y = yStartOffset; y < realBounds.h; y++) {

                for(int x = xStartOffset; x < realBounds.w; x++) {

                    // Okay due to our bounds checking.
                    ExPop::Gfx::Pixel *pd = dst.getPixelFast(x + dstRect.x, y + dstRect.y);

                    // Okay only if we're sure about our backing texture image.
                    const ExPop::Gfx::Pixel *ps = src.getPixelFast(x + srcRect.x, y + srcRect.y);

                    *pd = *ps;
                }
            }
        }

        inline void imgBlitForText(
            const ExPop::Gfx::Image &src,
            const ExPop::Gfx::Rectangle &srcRect,
            ExPop::Gfx::Image &dst,
            const ExPop::Gfx::Rectangle &dstRect,
            const ExPop::Gfx::Image *mask,
            const ExPop::Gfx::Rectangle *maskRect)
        {
            ExPop::Gfx::Rectangle realBounds = dstRect;

            // Check if output is before the buffer on either axis.
            if(realBounds.x + realBounds.w <= 0) {
                return;
            }
            if(realBounds.y + realBounds.h <= 0) {
                return;
            }

            // Check if output starts off the end on either axis.
            if(realBounds.x >= int(dst.getWidth())) {
                return;
            }
            if(realBounds.y >= int(dst.getHeight())) {
                return;
            }

            // Clamp area if we're over the edge.
            if(realBounds.x + realBounds.w > int(dst.getWidth())) {
                realBounds.w = dst.getWidth() - realBounds.x;
            }
            if(realBounds.y + realBounds.h > int(dst.getHeight())) {
                realBounds.h = dst.getHeight() - realBounds.y;
            }

            // Handle the case of starting off the edge.
            int xStartOffset = 0;
            if(dstRect.x < 0) {
                xStartOffset = -dstRect.x;
            }
            int yStartOffset = 0;
            if(dstRect.y < 0) {
                yStartOffset = -dstRect.y;
            }

            for(int y = yStartOffset; y < realBounds.h; y++) {

                for(int x = xStartOffset; x < realBounds.w; x++) {

                    // Okay due to our bounds checking.
                    ExPop::Gfx::Pixel *pd =
                        dst.getPixelFast(x + dstRect.x, y + dstRect.y);

                    // Okay only if we're sure about our backing
                    // texture image.
                    const ExPop::Gfx::Pixel *ps =
                        src.getPixelFast(x + srcRect.x, y + srcRect.y);

                    // Okay only if we're sure about our font.
                    const ExPop::Gfx::Pixel *pm =
                        mask ? mask->getPixelFast(x + maskRect->x, y + maskRect->y) : nullptr;

                    if(pm->rgba.a > 0)
                    {
                        // *pd = *pm;

                        if(pm->rgba.r) {

                            // Use the color from the source
                            // background.
                            *pd = *ps;

                        } else if(pd->rgba.a != 255) {

                            // Use the black color straight from the
                            // mask.
                            *pd = *pm;

                        }
                    }
                }
            }
        }

        inline ExPop::Gfx::Image *generateOutlinedFontMask(
            ExPop::Gfx::Image *font)
        {
            int characterWidth = font->getWidth() / 16;
            int characterHeight = font->getHeight() / 16;

            // Create a new image with the space to represent each
            // character with an extra pixel on each side.
            int outputCharacterWidth = characterWidth + 2;
            int outputCharacterHeight = characterHeight + 2;
            Image *output = new Image(
                outputCharacterWidth * 16,
                outputCharacterHeight * 16);
            output->clear();

            // Copy characters over.
            for(size_t y = 0; y < 16; y++) {
                for(size_t x = 0; x < 16; x++) {

                    Rectangle srcRect;
                    srcRect.x = characterWidth * x;
                    srcRect.y = characterHeight * y;
                    srcRect.w = characterWidth;
                    srcRect.h = characterHeight;

                    Rectangle dstRect;
                    dstRect.x = outputCharacterWidth * x + 1;
                    dstRect.y = outputCharacterHeight * y + 1;
                    dstRect.w = outputCharacterWidth - 2;
                    dstRect.h = outputCharacterHeight - 2;

                    imgBlit(*font, srcRect, *output, dstRect);
                }
            }

            // Clean up black areas.
            makeBlackTransparent(*output);

            // Add outline.
            for(int y = 0; y < int(output->getHeight()); y++) {
                for(int x = 0; x < int(output->getWidth()); x++) {

                    Pixel *outp = output->getPixelFast(x, y);
                    if(outp->rgba.r == 255 && outp->rgba.a) {

                        // Iterate through all adjacent pixels and set
                        // them black if there's nothing in them
                        // already.
                        for(int i = 0; i < 4; i++) {
                            int pos[2] = { x, y };
                            pos[i&1] += -1 + (i&2);

                            Pixel *outp2 =
                                output->getPixel(
                                    pos[0], pos[1]);

                            if(outp2 && outp2->rgba.a == 0) {
                                outp2->rgba.r = 0;
                                outp2->rgba.a = 255;
                            }

                        }

                    }
                }
            }

            return output;
        }

        inline void drawText(
            Image *font,
            Image *dst,
            Image *baseTex,
            int x, int y,
            const std::string &str,
            int lineSpacing,
            int charSpacing)
        {
            uint32_t charHeight = font->getHeight() / 16;
            uint32_t charWidth  = font->getWidth() / 16;
            uint32_t tabWidth   = 4;

            int scale = 1;

            int xoffset = x;
            int yoffset = y;
            uint32_t spacesThisLine = 0;

            std::vector<std::string> lines;
            stringTokenize(str, "\n", lines, true);

            for(size_t row = 0; row < lines.size(); row++) {

                // Early-out when we've gone past the end of the
                // image.
                if(yoffset >= int(dst->getHeight())) {
                    return;
                }

                for(size_t col = 0; col < lines[row].size(); col++) {

                    // Early-out when we've gone past the end of the
                    // image.
                    if(xoffset >= int(dst->getWidth())) {
                        break;
                    }

                    uint32_t c = lines[row][col];

                    if(c == '\t') {

                        // Tab. Insert spaces up to the next multiple
                        // of four spaces.
                        do {
                            xoffset += (charWidth + charSpacing) * scale;
                            spacesThisLine++;
                        } while(spacesThisLine % tabWidth);

                    } else if(c == ' ') {

                        // Don't bother rendering spaces. Just advance
                        // the rendering point.
                        xoffset += (charWidth + charSpacing) * scale;
                        spacesThisLine++;

                    } else {

                        // Normal letter. Render this.

                        Rectangle srcRect;
                        srcRect.x = (c & 0x0f) * charWidth;
                        srcRect.y = ((c & 0xf0) >> 4) * charHeight;
                        srcRect.w = charWidth;
                        srcRect.h = charHeight;

                        Rectangle dstRect;
                        dstRect.x = xoffset;
                        dstRect.y = yoffset;
                        dstRect.w = srcRect.w * scale;
                        dstRect.h = srcRect.h * scale;

                        if(baseTex) {

                            Rectangle maskRect;
                            maskRect.x = 0;
                            maskRect.y = 0;
                            maskRect.w = baseTex->getWidth();
                            maskRect.h = baseTex->getHeight();

                            imgBlitForText(
                                *baseTex, maskRect,
                                *dst, dstRect,
                                font, &srcRect);

                        } else {

                            imgBlitForText(
                                *font, srcRect,
                                *dst, dstRect,
                                font, &srcRect);

                        }

                        xoffset += (charWidth + charSpacing) * scale;
                        spacesThisLine++;
                    }
                }

                // Move onto next line.
                yoffset += (charHeight + lineSpacing) * scale;
                xoffset = x;
                spacesThisLine = 0;
            }
        }

    }
}

