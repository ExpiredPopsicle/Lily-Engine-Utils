// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
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

#pragma once

#include <ostream>
#include <vector>
#include <string>
#include <map>

namespace ExPop {

    /// Input to the preprocessor, and manager of state inside of it.
    class PreprocessorState {
    public:

        PreprocessorState(void);

        // ----------------------------------------------------------------------
        // Settings stuff
        // ----------------------------------------------------------------------

        /// Fill this with constants before starting. It'll also change as
        /// things get #defined and #undefed.
        std::map<std::string, std::string> definedSymbols;

        /// Include search paths.
        std::vector<std::string> includePaths;

        /// If this is set to true, all #includes will be accompanied with
        /// annotations as C++ style comments indicating that some file is
        /// being #included.
        bool annotateIncludes;

        // TODO: Maybe something to limit the include file options? Like a
        // whitelist of includable files.

        /// File loader callback type and variable. Defaults to
        /// ExPop::FileSystem::loadFileString().
        typedef std::string(*LoadFileCallback)(const std::string &filename);
        LoadFileCallback loadFileCallback;

        // ----------------------------------------------------------------------
        // Error reporting
        // ----------------------------------------------------------------------

        /// If this is set to true when the preprocessor returns, there
        /// was some kind of error.
        bool hadError;

        /// This contains the text of the error or errors.
        std::string errorText;

        // ----------------------------------------------------------------------
        // Internal state stuff
        // ----------------------------------------------------------------------

        /// Internally used to make sure we don't infinitely recurse with
        /// #include or something.
        unsigned int recursionCount;

        /// All included files we've hit so far.
        std::map<std::string, bool> fileList;

    private:
    };

    /// Run the preprocessor on a given string and return the
    /// processed string. fileName is for tracking errors and
    /// includes. Does not actually open the file on its own.
    std::string preprocess(
        const std::string &fileName,
        const std::string &inStr,
        PreprocessorState &inState);




    /// THIS IS THE OLD VERSION. DON'T USE THIS UNLESS YOU HAVE A GOOD
    /// REASON TO.

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
    ///   left as NULL, it won't output errors.

    void simplePreprocess(
        std::vector<std::string> &allFileNames,
        const std::string &fileName,
        const std::vector<std::string> &input,
        std::vector<std::string> &output,
        const std::vector<std::string> *inputConstants = NULL,
        std::ostream *errorStream = NULL,
        int recursionLevel = 0,
        const std::vector<std::string> *includePaths = NULL);


}

