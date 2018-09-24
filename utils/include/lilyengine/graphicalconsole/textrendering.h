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

// Simple text rendering (ASCII only, fixed-width only) that renders
// to an PixelImage<uint8_t>.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "../matrix.h"

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
            PixelImage<uint8_t> *font,
            PixelImage<uint8_t> *dst,
            PixelImage<uint8_t> *baseTex,
            int x, int y,
            const std::string &str,
            int lineSpacing = 0,
            int charSpacing = 0);

        /// Make all the black areas of an image transparent. This is
        /// basically just cheap color keying.
        void makeBlackTransparent(
            PixelImage<uint8_t> &img);

        /// Generate an outlined font for better reading on complex
        /// backgrounds. This will generate a bigger image to make
        /// room for the one-pixel outline.
        PixelImage<uint8_t> *generateOutlinedFontMask(
            const PixelImage<uint8_t> &font);

        /// Load up a new instance of the built-in font.
        PixelImage<uint8_t> *loadBuiltinFont();
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
            PixelImage<uint8_t> &img)
        {
            for(PixelImage_Coordinate y = 0; y < img.getHeight(); y++) {
                for(PixelImage_Coordinate x = 0; x < img.getWidth(); x++) {
                    PixelValue<uint8_t> *outp = &img.getData(x, y, 0);
                    if(outp[0].value == 0) {
                        outp[3].value = 0;
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
            const PixelImage<uint8_t> &src,
            const Gfx::Rectangle &srcRect,
            PixelImage<uint8_t> &dst,
            const Gfx::Rectangle &dstRect)
        {
            Gfx::Rectangle realBounds = dstRect;

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
                    PixelValue<uint8_t> *pd = &dst.getData(x + dstRect.x, y + dstRect.y, 0);

                    // Okay only if we're sure about our backing texture image.
                    const PixelValue<uint8_t> *ps = &src.getData(x + srcRect.x, y + srcRect.y, 0);

                    pd[0] = ps[0];
                    pd[1] = ps[1];
                    pd[2] = ps[2];
                    pd[3] = ps[3];
                }
            }
        }

        inline void imgBlitForText(
            const PixelImage<uint8_t> &src,
            const Gfx::Rectangle &srcRect,
            PixelImage<uint8_t> &dst,
            const Gfx::Rectangle &dstRect,
            const PixelImage<uint8_t> *mask,
            const Gfx::Rectangle *maskRect)
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
                    PixelValue<uint8_t> *pd =
                        &dst.getData(x + dstRect.x, y + dstRect.y, 0);

                    // Okay only if we're sure about our backing
                    // texture image.
                    const PixelValue<uint8_t> *ps =
                        &src.getData(x + srcRect.x, y + srcRect.y, 0);

                    // Okay only if we're sure about our font.
                    const PixelValue<uint8_t> *pm =
                        mask ? &mask->getData(x + maskRect->x, y + maskRect->y, 0) : nullptr;

                    if(pm[3].value > 0)
                    {
                        // *pd = *pm;

                        if(pm[0].value) {

                            // Use the color from the source
                            // background.
                            pd[0].value = ps[0].value;
                            pd[1].value = ps[1].value;
                            pd[2].value = ps[2].value;
                            pd[3].value = ps[3].value;

                        } else if(pd[3].value != 255) {

                            // Use the black color straight from the
                            // mask.
                            pd[0].value = pm[0].value;
                            pd[1].value = pm[1].value;
                            pd[2].value = pm[2].value;
                            pd[3].value = pm[3].value;

                        }
                    }
                }
            }
        }

        inline PixelImage<uint8_t> *generateOutlinedFontMask(
            const PixelImage<uint8_t> &font)
        {
            int characterWidth = font.getWidth() / 16;
            int characterHeight = font.getHeight() / 16;

            // Create a new image with the space to represent each
            // character with an extra pixel on each side.
            int outputCharacterWidth = characterWidth + 2;
            int outputCharacterHeight = characterHeight + 2;

            PixelImage<uint8_t> *output = new PixelImage<uint8_t>(
                outputCharacterWidth * 16,
                outputCharacterHeight * 16,
                4);

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

                    imgBlit(font, srcRect, *output, dstRect);
                }
            }

            // Clean up black areas.
            makeBlackTransparent(*output);

            // Add outline.
            for(int y = 0; y < int(output->getHeight()); y++) {
                for(int x = 0; x < int(output->getWidth()); x++) {

                    PixelValue<uint8_t> *outp = &output->getData(x, y, 0);
                    if(outp[0].value == 255 && outp[3]) {

                        // Iterate through all adjacent pixels and set
                        // them black if there's nothing in them
                        // already.
                        for(int i = 0; i < 4; i++) {
                            int pos[2] = { x, y };
                            pos[i&1] += -1 + (i&2);

                            PixelValue<uint8_t> *outp2 =
                                &output->getData(
                                    pos[0], pos[1], 0);

                            if(outp2 && outp2[3].value == 0) {
                                outp2[0].value = 0;
                                outp2[3].value = 255;
                            }

                        }

                    }
                }
            }

            return output;
        }

        inline void drawText(
            PixelImage<uint8_t> *font,
            PixelImage<uint8_t> *dst,
            PixelImage<uint8_t> *baseTex,
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

    inline PixelImage<uint8_t> *loadBuiltinFont()
    {
        size_t tgaFileLengthZlib = 0;
        uint8_t *tgaFileDataZlib = ExPop::Gfx::graphicalConsoleGetFontFileBuffer(&tgaFileLengthZlib);
        std::string copiedZlibData;
        copiedZlibData.resize(tgaFileLengthZlib);
        memcpy(&copiedZlibData[0], tgaFileDataZlib, tgaFileLengthZlib);
        std::string fontTgaBuffer;
        ExPop::Deflate::decompress_zlib(copiedZlibData, fontTgaBuffer);

        PixelImage<uint8_t> *fontImg = pixelImageLoadTGA(
            &fontTgaBuffer[0], fontTgaBuffer.size());

        // If we hit this, then we messed up the font image encoding
        // somehow.
        assert(fontImg);

        Gfx::makeBlackTransparent(*fontImg);

        return fontImg;
    }

}


