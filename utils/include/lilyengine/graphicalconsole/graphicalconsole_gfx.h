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

// GraphicalConsole internal implementation.

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#pragma once

#include "../pixelimage/pixelimage.h"
#include "textrendering.h"

namespace ExPop
{

    // FIXME: Rename this.
    inline PixelImage<uint8_t> *makeGradientImage(
        int blankSize,
        const ExPop::FVec3 &colorTop,
        const ExPop::FVec3 &colorMiddle,
        const ExPop::FVec3 &colorBottom)
    {
        // FIXME: Gradient fudge factor because I made the font too tall.
        // This is just the number of pixels on the top and bottom of
        // characters to ignore for gradient alpha calculation.
        int gradientSquish = 2;

        ExPop::PixelImage<uint8_t> *gradImg =
            new ExPop::PixelImage<uint8_t>(blankSize, blankSize, 4);

        for(int y = 0; y < blankSize; y++) {

            for(int x = 0; x < blankSize; x++) {

                PixelValue<uint8_t> *p = &gradImg->getData(x, y, 0);

                // Calculate alpha.
                int grad_y = y - gradientSquish;
                float a = float(grad_y) / float(blankSize - gradientSquish * 2);
                if(a < 0.0f) a = 0.0f;
                if(a > 1.0f) a = 1.0f;

                ExPop::FVec3 color;
                if(a < 0.5f) {
                    a *= 2.0f;
                    color = (colorTop * (1.0f - a)) +
                        (colorMiddle * a);
                } else {
                    a -= 0.5f;
                    a *= 2.0f;
                    color = (colorMiddle * (1.0f - a)) +
                        (colorBottom * a);
                }

                p[3] = 1.0f;

                // Clamp to sane range.
                for(int k = 0; k < 3; k++) {
                    if(color[k] > 1.0f) color[k] = 1.0f;
                    if(color[k] < 0.0f) color[k] = 0.0f;
                }

                p[0] = color[0];
                p[1] = color[1];
                p[2] = color[2];
            }
        }

        return gradImg;
    }


    enum {
        GRAPHICALCONSOLE_VT100_BIT_BRIGHT    = 00400,
        GRAPHICALCONSOLE_VT100_BIT_DIM       = 00200,
        GRAPHICALCONSOLE_VT100_BIT_UNDERLINE = 00100,
        GRAPHICALCONSOLE_VT100_MASK_FGCOLOR  = 00070,
        GRAPHICALCONSOLE_VT100_MASK_BGCOLOR  = 00007,

        // This one just means that things haven't been changed at all.
        GRAPHICALCONSOLE_VT100_BIT_DEFAULT   = 01000
    };

    inline void graphicalConsoleSplitTextByColor(
        const std::string &text,
        std::map<uint32_t, std::string> &output)
    {
        // Output index is bits...
        //   0 - "default"  (01000)
        //   1 - bright     (00400)
        //   2 - dim        (00200)
        //   3 - underscore (00100) (not used yet)
        // 567 - foreground color
        // 89(10) - background color (not used yet)

        // Base attributes: Not bright. Not dim. Not underscore. White. No
        // background.
        const uint32_t baseAttributes = 01077;

        uint32_t currentAttributes = baseAttributes;

        // This is to track the actual output character count, NOT
        // including the stuff eaten by the VT100 code "parser".
        uint32_t charCounter = 0;

        for(size_t i = 0; i < text.size(); i++) {

            if(text[i] == 0x1b) {

                // Skip '\e'.
                i++;

                if(i < text.size() && text[i] == '[') {

                    // Skip '['.
                    i++;

                    // Find the end of the color code. Look for something
                    // that's not a number or a semicolon.
                    size_t k = i;
                    for(; k < text.size(); k++) {
                        if((text[k] < '0' || text[k] > '9') && text[k] != ';') {
                            break;
                        }
                    }

                    // Split up the attribute numbers into individual
                    // strings.
                    std::string attribSubstr = text.substr(i, k - i);
                    std::vector<std::string> splitAttribs;
                    ExPop::stringTokenize(attribSubstr, ";", splitAttribs);

                    for(size_t n = 0; n < splitAttribs.size(); n++) {

                        // Convert attribute number string to an int.
                        uint32_t splitNum = 0;
                        std::istringstream istr(splitAttribs[n]);
                        istr >> splitNum;

                        if(splitNum >= 30 && splitNum <= 37) {

                            // Foreground color.

                            // Mask out fg color.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_MASK_FGCOLOR;

                            // OR back in the correct color.
                            currentAttributes |= (splitNum - 30) << 3;

                            // Unset default.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_BIT_DEFAULT;

                        } else if(splitNum >= 40 && splitNum <= 47) {

                            // Background color.

                            // Mask out bg color.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_MASK_BGCOLOR;

                            // OR back in the correct color.
                            currentAttributes |= splitNum - 40;

                            // Unset default.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_BIT_DEFAULT;

                        } else if(splitNum == 1) {

                            // Bright.
                            currentAttributes |= GRAPHICALCONSOLE_VT100_BIT_BRIGHT;

                            // Unset default.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_BIT_DEFAULT;

                        } else if(splitNum == 2) {

                            // Dim.
                            currentAttributes |= GRAPHICALCONSOLE_VT100_BIT_DIM;

                            // Unset default.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_BIT_DEFAULT;

                        } else if(splitNum == 4) {

                            // Underline.
                            currentAttributes |= GRAPHICALCONSOLE_VT100_BIT_UNDERLINE;

                            // Unset default.
                            currentAttributes = currentAttributes & ~GRAPHICALCONSOLE_VT100_BIT_DEFAULT;

                        } else if(splitNum == 0) {

                            // Reset.
                            currentAttributes = baseAttributes;

                        }

                    }

                    i = k;
                }

            } else {

                // Output a normal character.

                if(output[currentAttributes].size() <= charCounter) {
                    output[currentAttributes].resize(charCounter + 1, ' ');
                }

                output[currentAttributes][charCounter] = text[i];
                charCounter++;
            }

        }
    }


    inline void GraphicalConsole::updateBackbuffer(bool force)
    {
        if(!backBufferIsDirty && !force) {
            return;
        }

      #if EXPOP_ENABLE_THREADS
        lineRingBufferMutex.lock();
      #endif

        // Make a transparent checkered backround.
        uint8_t bgcolor[4] = {0};
        bgcolor[2] = 0x20;
        bgcolor[3] = bgAlpha * 255;

        uint8_t bgcolor2[4] = {0};
        bgcolor2[2] = 0x40;
        bgcolor2[3] = bgAlpha * 255;

        const size_t checkerSpacing = 16;
        for(PixelImage_Coordinate y = 0; y < backBuffer->getHeight(); y++) {
            for(PixelImage_Coordinate x = 0; x < backBuffer->getWidth(); x++) {

                bool b = ((y / checkerSpacing) + (x / checkerSpacing)) % 2;
                PixelValue<uint8_t> *val = &backBuffer->getData(x, y, 0);

                if(b) {
                    val[0].value = bgcolor[0];
                    val[1].value = bgcolor[1];
                    val[2].value = bgcolor[2];
                    val[3].value = bgcolor[3];
                } else {
                    val[0].value = bgcolor2[0];
                    val[1].value = bgcolor2[1];
                    val[2].value = bgcolor2[2];
                    val[3].value = bgcolor2[3];
                }
            }
        }

        // Start before the current index. Mask off bits to keep us in
        // range.
        size_t i =
            (lineRingBufferIndex - 1 - viewOffset) & (lineRingBufferSize - 1);

        int padding = 8;

        // Fudge factor to get rows more compact if the font is too big or
        // has too much empty space..
        int rowSquish = 6;
        int characterSquish = 2;

        int widthOfCharacter = outlinedFontImg->getWidth() / 16 - characterSquish;
        int rowLengthInPixels = backBuffer->getWidth() - 2 * padding;
        int rowLengthInCharacters = (rowLengthInPixels / widthOfCharacter);

        // Start at the very bottom, minus any padding. First iteration
        // will move us up by one row before starting.
        int pixelRow = backBuffer->getHeight() - rowSquish - padding;
        int rowSize = (outlinedFontImg->getHeight() / 16) - rowSquish;

        // Offset pixelRow up one more line so we have somewhere to enter
        // text.
        pixelRow -= rowSize;

        // Draw console text.
        while(i != lineRingBufferIndex) {

            std::string &lineText = lineRingBuffer[i];

            pixelRow -= rowSize;

            std::string lineTextToPrint = lineText;
            if(int(lineText.size()) > rowLengthInCharacters) {

                // FIXME: Not color-code aware!
                // FIXME: This strips out a lot of spacing.
                lineTextToPrint = ExPop::stringWordWrap(
                    lineText,
                    std::max(0, rowLengthInCharacters),
                    std::max(0, rowLengthInCharacters - 4));

                lineTextToPrint = ExPop::stringIndent(
                    lineTextToPrint,
                    0, 4);
            }

            size_t newlineCount = 0;

            // Count newlines.
            for(size_t n = 0; n < lineTextToPrint.size(); n++) {
                if(lineTextToPrint[n] == '\n') {
                    newlineCount++;
                }
            }

            pixelRow -= newlineCount * rowSize;

            if(pixelRow + int(newlineCount + 1) * rowSize < 0) {
                // This line will be entirely off the screen. No reason to
                // draw this or keep drawing more.
                break;
            }

            // Split into individual draws by color.
            std::map<uint32_t, std::string> stringsByDisplayAttrib;
            graphicalConsoleSplitTextByColor(lineTextToPrint, stringsByDisplayAttrib);
            for(auto k = stringsByDisplayAttrib.begin(); k != stringsByDisplayAttrib.end(); k++) {

                // The gradientsByColor array is 3 sets of the 8 colors,
                // corresponding to 3 levels of brightness. Start off in
                // the middle set (color value + 8).
                uint32_t foregroundColor = ((k->first & GRAPHICALCONSOLE_VT100_MASK_FGCOLOR) >> 3) + 8;

                // Now add or subtract on set's worth for bright and dim
                // bits.
                if(k->first & GRAPHICALCONSOLE_VT100_BIT_BRIGHT) {
                    foregroundColor += 8;
                }
                if(k->first & GRAPHICALCONSOLE_VT100_BIT_DIM) {
                    foregroundColor -= 8;
                }

                Gfx::drawText(
                    outlinedFontImg,
                    backBuffer,
                    !!(k->first & GRAPHICALCONSOLE_VT100_BIT_DEFAULT) ? gradImg : gradientsByColor[foregroundColor],
                    padding, pixelRow,
                    k->second,
                    -rowSquish, -characterSquish);
            }

            // Next line.
            i--;
            i = i & (lineRingBufferSize - 1);
        }

        // ----------------------------------------------------------------------
        // Text input display.

        std::ostringstream promptData;

        // if(viewOffset != 0) {
        //     promptData << "(Line: -" << viewOffset << ") ";
        // }
        // if(historyPosition != -1) {
        //     promptData << "(History: " << historyPosition << ") ";
        // }

        promptData << prompt;
        std::string promptStr = promptData.str();
        std::string editLineWithPrompt =
            promptStr + stringUTF32ToCodepage437(editLineBuffer);

        int displayOffset = 0;
        if(int(editLineCursorLocation + promptStr.size()) >= rowLengthInCharacters) {
            displayOffset = 1 + editLineCursorLocation + promptStr.size() - rowLengthInCharacters;
        }

        int inputAreaYOffset = backBuffer->getHeight() - rowSquish - padding - rowSize;

        // Draw the text input area.
        Gfx::drawText(
            outlinedFontImg,
            backBuffer,
            // gradImg,
            gradientsByColor[7+16], // Bright white
            padding,
            inputAreaYOffset,
            editLineWithPrompt.substr(displayOffset, rowLengthInCharacters),
            -rowSquish, -characterSquish);

        // Draw cursor. This is kind of a weird hack. We're just going to
        // render a string with a bunch of spaces and then our cursor
        // character.
        std::string spacesWithCursor;
        int numSpaces = editLineCursorLocation + promptStr.size();
        spacesWithCursor.resize(numSpaces + 1);
        memset(&spacesWithCursor[0], ' ', numSpaces);
        spacesWithCursor[numSpaces] = 219; // Extended ASCII full block.

        Gfx::drawText(
            outlinedFontImg,
            backBuffer,
            gradientsByColor[7+16], // Bright white
            padding,
            inputAreaYOffset,
            spacesWithCursor.substr(displayOffset),
            -rowSquish, -characterSquish);

        // If the cursor is over something already in the string, we
        // should re-draw that character as an inverse color on top of the
        // cursor.
        if(editLineCursorLocation < int(editLineBuffer.size())) {

            spacesWithCursor[numSpaces] =
                editLineBuffer[editLineCursorLocation];

            Gfx::drawText(
                outlinedFontImg,
                backBuffer,
                gradientsByColor[0], // Black
                padding,
                inputAreaYOffset,
                spacesWithCursor.substr(displayOffset),
                -rowSquish, -characterSquish);
        }

        backBufferIsDirty = false;

        backbufferUpdateCount++;

      #if EXPOP_ENABLE_THREADS
        lineRingBufferMutex.unlock();
      #endif
    }

}
