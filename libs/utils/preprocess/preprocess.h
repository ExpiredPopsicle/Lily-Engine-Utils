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

#pragma once

#include <ostream>
#include <vector>
#include <string>

namespace ExPop {

    /// Handle some simple (not compliant) C preprocessor style
    /// directives. Currently parses #include directives.

    /// allFileNames is a list of strings that will be filled with the
    ///   names of all the files that were eventually included.

    /// fileName is the name of the current file (for the purposes of
    ///   dealing with #includes and errors).

    /// input is the string to be processed.

    /// output is the string to fill with the processed data.

    /// inputConstants is an optional vector of strings indicating
    ///   #defines to stick at the top of the processed output.

    /// errorStream is an output stream to send errors to. If it is
    ///   left as NULL, it will use cout.

    void simplePreprocess(
        std::vector<std::string> &allFileNames,
        const std::string &fileName,
        const std::vector<std::string> &input,
        std::vector<std::string> &output,
        const std::vector<std::string> *inputConstants = NULL,
        std::ostream *errorStream = NULL,
        int recursionLevel = 0);

}

