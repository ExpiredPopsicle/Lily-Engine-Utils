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

    inline std::ostream &out()
    {
        // Fall back on std::cout if the main console has already been
        // cleaned up. This is to handle the case of global/static
        // destructors firing off after the main GraphicalConsole's
        // destructor has already gone so we don't write to a dead
        // console.
        if(!getMainConsoleAvailable()) {
            return std::cout;
        }

        return getMainConsole()->out;
    }

    inline GraphicalConsole::GraphicalConsoleStreambuf::GraphicalConsoleStreambuf(GraphicalConsole *inParent)
    {
        parent = inParent;
    }

    inline int GraphicalConsole::GraphicalConsoleStreambuf::overflow(int c)
    {
      #if EXPOP_ENABLE_THREADS
        buffersMutex.lock();
        Threads::ThreadId thisId = Threads::getMyId();
        std::string &threadBuffer = buffersByThread[thisId];
      #else
        std::string &threadBuffer = singleThreadedBuffer;
      #endif

        if(c != '\n') {
            threadBuffer.append(1, c);
        } else {
            parent->addLine(threadBuffer);
            threadBuffer.clear();
        }

      #if EXPOP_ENABLE_THREADS
        buffersMutex.unlock();
      #endif

        return c;
    }

}
