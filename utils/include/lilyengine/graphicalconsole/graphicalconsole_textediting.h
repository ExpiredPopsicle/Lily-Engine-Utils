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

namespace ExPop
{

    inline void GraphicalConsole::editMoveCursor(int amount)
    {
        if(!amount) {
            return;
        }

        editLineCursorLocation += amount;

        if(editLineCursorLocation < 0) {
            editLineCursorLocation = 0;
        }

        if(editLineCursorLocation > int(editLineBuffer.size())) {
            editLineCursorLocation = int(editLineBuffer.size());
        }

        backBufferIsDirty = true;
    }

    inline void GraphicalConsole::editMoveCursorWord(int amount)
    {
        if(amount == 0) return;

        // Split up amount and direction.
        int direction = ((amount > 0) * 2) - 1;
        amount = amount * direction;

        while(amount > 0) {

            // Early-out if we're already at the start or end of the line.
            if(direction + editLineCursorLocation < 0 ||
                direction + editLineCursorLocation > int(editLineBuffer.size()))
            {
                break;
            }

            // Skip stuff until we hit the end of the line, start of the
            // line, or a space.
            do {
                editLineCursorLocation += direction;
            } while(
                editLineCursorLocation > 0 && // Hit start.
                editLineCursorLocation < int(editLineBuffer.size()) && // Hit end.
                editLineBuffer[editLineCursorLocation] != ' '); // Hit space.

            amount--;
        }

        backBufferIsDirty = true;
    }

    inline void GraphicalConsole::editMoveToEdge(int direction)
    {
        if(direction < 0) {
            editLineCursorLocation = 0;
            backBufferIsDirty = true;
        } else if(direction > 0) {
            editLineCursorLocation = editLineBuffer.size();
            backBufferIsDirty = true;
        }
    }

    inline void GraphicalConsole::editInsertText(const std::string &utf8Text)
    {
        historyPosition = -1;

        std::basic_string<uint32_t> newData = ExPop::stringUTF8ToUTF32(utf8Text);

        editLineBuffer.insert(
            editLineBuffer.begin() + editLineCursorLocation,
            newData.begin(),
            newData.end());

        editMoveCursor(newData.size());

        backBufferIsDirty = true;
    }

    inline void GraphicalConsole::editDeleteText(size_t amount)
    {
        historyPosition = -1;

        if(editLineCursorLocation < int(editLineBuffer.size())) {

            if(amount > editLineBuffer.size() - editLineCursorLocation) {
                amount = editLineBuffer.size() - editLineCursorLocation;
            }

            editLineBuffer.erase(
                editLineBuffer.begin() + editLineCursorLocation,
                editLineBuffer.begin() + editLineCursorLocation + amount);

            backBufferIsDirty = true;
        }
    }

    inline void GraphicalConsole::displayMovePage(int amount)
    {
        viewOffset += amount;
        if(viewOffset < 0) {
            viewOffset = 0;
        }

        backBufferIsDirty = true;
    }

    inline const PixelImage<uint8_t> *GraphicalConsole::getBackbuffer() const
    {
        return backBuffer;
    }

    inline void GraphicalConsole::editMoveHistory(int direction)
    {
        backBufferIsDirty = true;

        if(direction < 0) {

            // Coming out of history mode, we'll just clear the line. Keep
            // in mind that if we go into history mode with something on
            // the line, it'll become part of the history. So we don't
            // have to worry about going back to that. It's already
            // handled.
            if(historyPosition == 0) {
                editLineBuffer.clear();
                editLineCursorLocation = 0;
                historyPosition = -1;
                return;
            }

            // If we're already in the non-history mode. Push THIS line as
            // new history, if it has anything in it.
            if(historyPosition < 0) {

                historyPosition = -1;

                if(editLineBuffer.size()) {
                    addHistory(editLineBuffer);
                }

                // Clear the line.
                editLineBuffer.clear();
                editLineCursorLocation = 0;

                return;
            }

        }

        if(direction > 0) {
            // Starting to traverse history. Save current line if there's
            // anything there.
            if(historyPosition < 0) {
                if(editLineBuffer.size()) {
                    addHistory(editLineBuffer);
                    historyPosition++; // Skip past what we just added.
                }
            }
        }

        if(commandHistory.size()) {

            // Move the history pointer and clamp it.
            historyPosition += direction;
            if(historyPosition >= int(commandHistory.size())) {
                historyPosition = commandHistory.size() - 1;
            }

            // Extract the history.
            editLineBuffer = commandHistory[historyPosition];
            editLineCursorLocation = editLineBuffer.size();

        }
    }

    inline void GraphicalConsole::addHistory(const std::basic_string<uint32_t> &newLine)
    {
        commandHistory.insert(commandHistory.begin(), newLine);

        if(commandHistory.size() > 64) {
            commandHistory.erase(commandHistory.end());
        }
    }

    inline void GraphicalConsole::editSubmitText()
    {
        historyPosition = -1;

        if(editLineBuffer.size()) {

            std::basic_string<uint32_t> line = editLineBuffer;

            // Add it to the history.
            addHistory(editLineBuffer);

            // Echo it back.
            addLine("\e[1m" + prompt + ExPop::stringUTF32ToUTF8(editLineBuffer));
            editLineBuffer.clear();
            editLineCursorLocation = 0;

            runCommand(line);
        }

        backBufferIsDirty = true;
    }

}
