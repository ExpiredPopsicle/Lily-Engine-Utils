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

// C/C++ style preprocessor. This is mainly for shaders. Does not
// support expressions in your #ifs. Does support "#pragma once".
// Supports extra non-standard tags "#ppenable" and "#ppdisable" to
// pass-through preprocessor tags.

// Do not use on untrusted code, unless you want the user to '#include
// "/etc/shadow"'.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iostream>
#include <ostream>
#include <vector>
#include <string>
#include <map>

#include "malstring.h"
#include "filesystem.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Input to the preprocessor, and manager of state inside of it.
    class PreprocessorState
    {
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

        // FIXME: Add a userdata thing to the file loader.

        /// File loader callback type and variable. Defaults to
        /// a simple file loader.
        typedef bool (*LoadFileCallback)(
            const std::string &filename,
            std::string &filedata);

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
    /// includes. Does not actually open the file on its own. Do not
    /// run on untrusted code!
    std::string preprocess(
        const std::string &fileName,
        const std::string &inStr,
        PreprocessorState &inState);

}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    const size_t RECURSION_LIMIT = 64;
    const size_t MAX_SYMBOL_LOOKUPS_FOR_IF = 64;

    inline bool preprocessorState_defaultLoadFileCallback(
        const std::string &filename,
        std::string &filedata)
    {
        int64_t fileLength = 0;
        char *rawFileData = FileSystem::loadFile(filename, &fileLength);

        if(rawFileData) {
            filedata = std::string(rawFileData, fileLength);
            return true;
        }
        return false;
    }

    inline PreprocessorState::PreprocessorState(void)
    {
        recursionCount = 0;
        hadError = false;
        annotateIncludes = false;
        loadFileCallback = preprocessorState_defaultLoadFileCallback;
    }

    inline std::string convertQuotedPath(const std::string &in)
    {
        if(in.size() < 2) return "";

        std::string workStr;

        if((in[0] == '"' && in[in.size()-1] == '"') ||
           (in[0] == '<' && in[in.size()-1] == '>')) {
            workStr = stringUnescape(in.substr(1, in.size() - 2));
        } else {
            workStr = in;
        }

        return workStr;
    }

    inline std::string cppItoa(int i)
    {
        std::ostringstream ostr;
        ostr << i;
        return ostr.str();
    }

  #define EXPOP_PP_CHECK_OR_ERROR(y, x) {                               \
        if(!(y)) {                                                      \
            inState.hadError = true;                                    \
            inState.errorText = inState.errorText + fileName + ":" + cppItoa(i) + " " + (x) + "\n"; \
            return "";                                                  \
        }                                                               \
    }

    /// This function is explicitly NOT safe to run untrusted code
    /// because there are no limits to the paths used on #include
    /// directives.
    inline std::string preprocess(
        const std::string &fileName,
        const std::string &inStr,
        PreprocessorState &inState)
    {
        // Set this file in the list of files we've already been to, but
        // keep a record of whether or not it was already in the list so
        // that we can bail out if we see a #pragma once.
        std::string fixedName = FileSystem::fixFileName(fileName);
        bool wasAlreadyInList = inState.fileList.count(fixedName) ? inState.fileList[fixedName] : false;
        inState.fileList[fixedName] = true;

        std::ostringstream ostr;

        std::vector<std::string> inLines;
        stringTokenize(inStr, "\n", inLines, true);

        // Number of #endifs we're expecting after passed tests in #if,
        // #ifdef, or #ifndef blocks.
        int nestedPassedIfs = 0;

        // Number of #endifs we need to see before we'll go back to
        // actually outputting stuff.
        int nestedFailedIfs = 0;

        // If this is disabled, just pass through everything, in case some
        // other preprocessor will run on code after this and we shouldn't
        // handle preprocessor junk bound for that. I'm looking at you,
        // GLSL.
        bool ppEnabled = true;

        for(unsigned int i = 0; i < inLines.size(); i++) {

            // Try to quickly find out if this line starts with a #.
            bool startsWithHash = false;
            unsigned int skipWhitespace = 0;
            while(skipWhitespace < inLines[i].size()) {
                if(isWhiteSpace(inLines[i][skipWhitespace])) {
                    skipWhitespace++;
                } else {
                    if(inLines[i][skipWhitespace] == '#') {
                        startsWithHash = true;
                    }
                    break;
                }
            }

            if(startsWithHash) {

                std::vector<std::string> tokens;
                stringTokenize(inLines[i], " \t", tokens, false);

                if(tokens.size()) {

                    if(tokens[0] == "#pragma" && ppEnabled) {

                        EXPOP_PP_CHECK_OR_ERROR(tokens.size() >= 2, "Bad #pragma.");

                        std::string pragmaType = tokens[1];

                        if(pragmaType == "once") {
                            if(wasAlreadyInList) {
                                if(inState.annotateIncludes) {
                                    return "// --- Skipping " + fixedName + " due to #pragma once.\n";
                                }
                                return "";
                            }
                        }

                    } else if(tokens[0] == "#define" && ppEnabled) {

                        if(!nestedFailedIfs) {

                            std::string symbol;
                            std::string value;

                            EXPOP_PP_CHECK_OR_ERROR(
                                tokens.size() == 2 || tokens.size() == 3,
                                "Bad #define.");

                            symbol = tokens[1];
                            if(tokens.size() >= 3) {
                                value = tokens[2];
                            }

                            EXPOP_PP_CHECK_OR_ERROR(
                                !inState.definedSymbols.count(symbol),
                                "#define for something that's already defined: \"" + symbol + "\".");

                            inState.definedSymbols[symbol] = value;
                        }

                    } else if(tokens[0] == "#undef" && ppEnabled) {

                        if(!nestedFailedIfs) {

                            EXPOP_PP_CHECK_OR_ERROR(
                                tokens.size() >= 2,
                                "Bad #undef");

                            std::string symbol;
                            symbol = tokens[1];

                            // TODO: Include the symbol in this error.
                            EXPOP_PP_CHECK_OR_ERROR(
                                inState.definedSymbols.count(symbol),
                                "#undef for symbol that doesn't exist: \"" + symbol + "\".");

                            inState.definedSymbols.erase(symbol);
                        }

                    } else if(tokens[0] == "#include" && ppEnabled) {

                        EXPOP_PP_CHECK_OR_ERROR(tokens.size() == 2, "Bad #include");

                        if(!nestedFailedIfs) {

                            EXPOP_PP_CHECK_OR_ERROR(
                                inState.recursionCount < RECURSION_LIMIT,
                                "Too many levels of #include recursion.");

                            std::string currentFilePath = FileSystem::getParentName(fileName);
                            if(!currentFilePath.size()) {
                                currentFilePath = ".";
                            }

                            std::string name = convertQuotedPath(tokens[1]);
                            std::string buf;
                            std::string fullName;

                            // First try loading from the same directory.
                            fullName = FileSystem::fixFileName(currentFilePath + "/" + name);
                            bool readSuccess = inState.loadFileCallback(fullName, buf);

                            if(!readSuccess) {

                                // Failed to load from current directory.
                                // Try include paths.
                                for(size_t i = 0; i < inState.includePaths.size(); i++) {
                                    fullName = FileSystem::fixFileName(inState.includePaths[i] + "/" + name);
                                    readSuccess = inState.loadFileCallback(fullName, buf);
                                    if(readSuccess) break;
                                }
                            }

                            EXPOP_PP_CHECK_OR_ERROR(
                                readSuccess,
                                "Could not open include file: " + fullName);

                            inState.recursionCount++;
                            std::string includedBuf = preprocess(fullName, buf, inState);
                            inState.recursionCount--;

                            EXPOP_PP_CHECK_OR_ERROR(
                                !inState.hadError,
                                "Error in included file: " + fullName);

                            // TODO: Error handling for recursive call.

                            if(inState.annotateIncludes) {
                                ostr << "// --- Begin include: " << fullName << std::endl;
                            }

                            ostr << includedBuf; // << std::endl;

                            if(inState.annotateIncludes) {
                                ostr << "// --- End include: " << fullName << std::endl;
                            }

                        }

                    } else if((tokens[0] == "#ifdef" || tokens[0] == "#ifndef" || tokens[0] == "#if") && ppEnabled) {

                        // Use the same code for #ifdef and #ifndef. Just
                        // flip the action if it's #ifndef instead of
                        // #ifdef.
                        bool flipResult = false;
                        if(tokens[0] == "#ifndef") {
                            flipResult = true;
                        }

                        if(nestedFailedIfs) {

                            // If we're already inside a failed if
                            // block, just increment the number of
                            // #endifs we need to see in order to get
                            // out of it.
                            nestedFailedIfs++;

                        } else {

                            EXPOP_PP_CHECK_OR_ERROR(
                                tokens.size() == 2,
                                "Bad #ifdef or #if");

                            std::string symbol = tokens[1];
                            bool result;

                            if(tokens[0] == "#if") {

                                // If we want to support actual
                                // expressions some day, this would be
                                // the place to do it. But right now
                                // we only support single symbol
                                // lookups.

                                std::string curSymbol = symbol;
                                size_t numLookups = 0;

                                // Keep looking up symbols until we find
                                // something that's not a symbol.
                                while(inState.definedSymbols.count(curSymbol)) {
                                    curSymbol = inState.definedSymbols[curSymbol];
                                    numLookups++;
                                    EXPOP_PP_CHECK_OR_ERROR(
                                        numLookups < MAX_SYMBOL_LOOKUPS_FOR_IF,
                                        "Too many lookups for symbol evaluation: \"" + symbol + "\".");
                                }

                                // Whatever this last thing is,
                                // convert it to an int. If it's
                                // nonzero, then the result is true.
                                int intVal = 0;
                                if(curSymbol.size()) {
                                    std::istringstream inStr(curSymbol);
                                    inStr >> intVal;
                                }
                                result = intVal;

                            } else {

                                result = inState.definedSymbols.count(symbol);

                            }

                            if(flipResult ? !result : result) {
                                nestedPassedIfs++;
                            } else {
                                nestedFailedIfs++;
                            }
                        }

                    } else if(tokens[0] == "#else" && ppEnabled) {

                        EXPOP_PP_CHECK_OR_ERROR(
                            nestedFailedIfs || nestedPassedIfs,
                            "Cannot use #else outside of a conditional block.");

                        if(nestedFailedIfs == 1) {
                            nestedFailedIfs--;
                            nestedPassedIfs++;
                        } else if(nestedPassedIfs) {
                            nestedPassedIfs--;
                            nestedFailedIfs++;
                        }

                    } else if(tokens[0] == "#endif" && ppEnabled) {

                        EXPOP_PP_CHECK_OR_ERROR(
                            nestedFailedIfs || nestedPassedIfs,
                            "Cannot use #endif without a conditional block.");

                        if(nestedFailedIfs) {
                            nestedFailedIfs--;
                        } else {
                            nestedPassedIfs--;
                        }

                    } else if(tokens[0] == "#ppenable") {

                        // Enable preprocessor.
                        if(!nestedFailedIfs) {
                            ppEnabled = true;
                        }

                    } else if(tokens[0] == "#ppdisable") {

                        // Disable preprocessor.
                        if(!nestedFailedIfs) {
                            ppEnabled = false;
                        }

                    } else {
                        if(!nestedFailedIfs) {
                            ostr << inLines[i] << std::endl;
                        }
                    }

                } else {
                    ostr << std::endl;
                }

            } else {

                // Didn't start with a # sign at all.
                if(!nestedFailedIfs) {
                    ostr << inLines[i] << std::endl;
                }
            }
        }

        return ostr.str();
    }
}

// FIXME:
//   Doesn't support userdata in callbacks.
//   Lots of bare members on the PreprocessorState class.
//   Why is "preprocess" a separate function? Just put it in the class.

