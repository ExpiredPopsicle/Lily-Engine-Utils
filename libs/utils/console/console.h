// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2010 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   Copyright (c) 2011 Clifford Jolly
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

// Thread-safe console output system interface. The external interface
// here is meant to be as simple as possible so we can just use it as
// a replacement for cout and cerr where appropriate. Also, it now has
// features for keeping the buffered data so we can send stuff to
// in-game graphical consoles using the same simple interface.

#pragma once

#include <vector>
#include <iostream>
#include <string>

namespace ExPop {

    namespace Console {

        /// Get an output stream. (Use like: out("something") << "blah" <<
        /// endl; )
        std::ostream &out(const std::string &outStreamName);

        /// Wrap a number to an output stream. In case you for some reason
        /// need to. It just converts to a string internally, though.
        std::ostream &out(int streamNum);

        /// Set VT100 attributes for a buffer. (The stuff that goes
        /// between the "<ESC>[" and the "m".) Each line is cleared with
        /// a "<ESC>[0m" after printing if this is set.
        void outSetAttribs(const std::string &outStreamName, const std::string &attribs);

        /// Set a prefix for a buffer. This will print immediately before a
        /// line's text. For example: "Error: " for an error output buffer.
        void outSetPrefix(const std::string &outStreamName, const std::string &prefix);

        /// Turn an output stream on or off. Note that Windows seems
        /// to suffer overhead from disabled consoles. So don't use
        /// this to just turn off some over-verbose system that's
        /// slowing everything down.
        void outSetEnabled(const std::string &outStreamName, bool enabled);

        /// Turn on synced (outputs to cout in calling thread) flag.
        void outSetSynced(const std::string &outStreamName, bool synced);

        /// Turn on or off support for graphical console output.
        void outSetGraphical(const std::string &outStreamName, bool graphical);

        /// Get new graphical console output lines. Anything output to
        /// a console with outSetGraphical will add to the list. The
        /// internal list will be cleared when this function is
        /// called.
        void outGetGraphicalLines(std::vector<std::string> &lines);

    }
}

