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

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
using namespace std;

#include "malstring.h"
#include "filesystem.h"
#include "preprocess.h"

namespace ExPop {

  #define RECURSION_LIMIT 64
  #define MAX_SYMBOL_LOOKUPS_FOR_IF 64

    PreprocessorState::PreprocessorState(void)
    {
        recursionCount = 0;
        hadError = false;
        annotateIncludes = false;
        loadFileCallback = ExPop::FileSystem::loadFileString;
    }

    static std::string convertQuotedPath(const std::string &in)
    {
        if(in.size() < 2) return "";

        string workStr;

        if((in[0] == '"' && in[in.size()-1] == '"') ||
           (in[0] == '<' && in[in.size()-1] == '>')) {
            workStr = stringUnescape(in.substr(1, in.size() - 2));
        } else {
            workStr = in;
        }

        return workStr;
    }

    static std::string cppItoa(int i)
    {
        ostringstream ostr;
        ostr << i;
        return ostr.str();
    }

  #define CHECK_OR_ERROR(y, x) {                                        \
        if(!(y)) {                                                      \
            inState.hadError = true;                                    \
            inState.errorText = inState.errorText + fileName + ":" + cppItoa(i) + " " + (x) + "\n"; \
            return "";                                                  \
        }                                                               \
    }

    /// This function is explicitly NOT safe to run untrusted code with
    /// because there are no limits to the paths used on #include
    /// directives.
    std::string preprocess(
        const std::string &fileName,
        const std::string &inStr,
        PreprocessorState &inState) {

        // Set this file in the list of files we've already been to, but
        // keep a record of whether or not it was already in the list so
        // that we can bail out if we see a #pragma once.
        string fixedName = FileSystem::fixFileName(fileName);
        bool wasAlreadyInList = inState.fileList.count(fixedName) ? inState.fileList[fixedName] : false;
        inState.fileList[fixedName] = true;

        ostringstream ostr;

        vector<string> inLines;
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

                vector<string> tokens;
                stringTokenize(inLines[i], " \t", tokens, false);

                if(tokens.size()) {

                    if(tokens[0] == "#pragma" && ppEnabled) {

                        CHECK_OR_ERROR(tokens.size() >= 2, "Bad #pragma.");

                        string pragmaType = tokens[1];

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

                            string symbol;
                            string value;

                            CHECK_OR_ERROR(
                                tokens.size() == 2 || tokens.size() == 3,
                                "Bad #define.");

                            symbol = tokens[1];
                            if(tokens.size() >= 3) {
                                value = tokens[2];
                            }

                            CHECK_OR_ERROR(
                                !inState.definedSymbols.count(symbol),
                                "#define for something that's already defined: \"" + symbol + "\".");

                            inState.definedSymbols[symbol] = value;
                        }

                    } else if(tokens[0] == "#undef" && ppEnabled) {

                        if(!nestedFailedIfs) {

                            CHECK_OR_ERROR(
                                tokens.size() >= 2,
                                "Bad #undef");

                            string symbol;
                            symbol = tokens[1];

                            // TODO: Include the symbol in this error.
                            CHECK_OR_ERROR(
                                inState.definedSymbols.count(symbol),
                                "#undef for symbol that doesn't exist: \"" + symbol + "\".");

                            inState.definedSymbols.erase(symbol);
                        }

                    } else if(tokens[0] == "#include" && ppEnabled) {

                        CHECK_OR_ERROR(tokens.size() == 2, "Bad #include");

                        if(!nestedFailedIfs) {

                            CHECK_OR_ERROR(
                                inState.recursionCount < RECURSION_LIMIT,
                                "Too many levels of #include recursion.");

                            string currentFilePath = FileSystem::getParentName(fileName);
                            if(!currentFilePath.size()) {
                                currentFilePath = ".";
                            }

                            string name = convertQuotedPath(tokens[1]);
                            std::string buf;
                            string fullName;

                            // First try loading from the same directory.
                            fullName = FileSystem::fixFileName(currentFilePath + "/" + name);
                            buf = inState.loadFileCallback(fullName);

                            if(!buf.size()) {

                                // Failed to load from current directory.
                                // Try include paths.
                                for(unsigned int i = 0; i < inState.includePaths.size(); i++) {
                                    fullName = FileSystem::fixFileName(inState.includePaths[i] + "/" + name);
                                    buf = inState.loadFileCallback(fullName);
                                    if(buf.size()) break;
                                }
                            }

                            // TODO: Get the name in there somewhere after
                            // I change this from just being asserts.
                            CHECK_OR_ERROR(buf.size(), "Could not open include file");

                            inState.recursionCount++;
                            string includedBuf = preprocess(fullName, buf, inState);
                            inState.recursionCount--;

                            CHECK_OR_ERROR(
                                !inState.hadError,
                                "Error in included file: " + fullName);

                            // TODO: Error handling for recursive call.

                            if(inState.annotateIncludes) {
                                ostr << "// --- Begin include: " << fullName << endl;
                            }

                            ostr << includedBuf; // << endl;

                            if(inState.annotateIncludes) {
                                ostr << "// --- End include: " << fullName << endl;
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

                            CHECK_OR_ERROR(
                                tokens.size() == 2,
                                "Bad #ifdef");

                            string symbol = tokens[1];
                            bool result;

                            if(tokens[0] == "#if") {

                                string curSymbol = symbol;
                                int numLookups = 0;

                                // Keep looking up symbols until we find
                                // something that's not a symbol.
                                while(inState.definedSymbols.count(curSymbol)) {
                                    curSymbol = inState.definedSymbols[curSymbol];
                                    numLookups++;
                                    CHECK_OR_ERROR(
                                        numLookups < MAX_SYMBOL_LOOKUPS_FOR_IF,
                                        "Too many lookups for symbol evaluation: \"" + symbol + "\".");
                                }

                                // Whatever this last thing is, convert it
                                // to an int.
                                int intVal = 0;
                                if(curSymbol.size()) {
                                    istringstream inStr(curSymbol);
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

                        CHECK_OR_ERROR(
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

                        CHECK_OR_ERROR(
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
                            ostr << inLines[i] << endl;
                        }
                    }

                } else {
                    ostr << endl;
                }

            } else {

                // Didn't start with a # sign at all.
                if(!nestedFailedIfs) {
                    ostr << inLines[i] << endl;
                }
            }
        }

        return ostr.str();
    }


    // ----------------------------------------------------------------------
    // Old, deprecated junk below.
    // ----------------------------------------------------------------------

    static unsigned int getFileNamePosition(
        vector<string> &allFileNames,
        const string &name) {

        for(unsigned int i = 0; i < allFileNames.size(); i++) {
            if(allFileNames[i] == name) {
                return i;
            }
        }

        allFileNames.push_back(name);
        return allFileNames.size() - 1;
    }

    static string getFileLineNumber(
        vector<string> &allFileNames,
        const string &name,
        unsigned int lineNumber) {

        ostringstream str;
        str <<
            "//#line " << lineNumber <<
            " " << getFileNamePosition(allFileNames, name) <<
            " // " << name;

        // This is disabled because it's borked on Intel shader
        // compiles.

        // return str.str();
        return "";
    }

    void simplePreprocess(
        vector<string> &allFileNames,
        const std::string &fileName,
        const vector<string> &input,
        vector<string> &output,
        const vector<string> *inputConstants,
        ostream *errorStream,
        int recursionLevel,
        const vector<string> *includePaths) {

        vector<string> currentFileOutput;

        // Figure out if we've included this file already before we
        // run into a pragma once.
        bool alreadyIncludedThis = false;
        for(unsigned int i = 0; i < allFileNames.size(); i++) {
            if(allFileNames[i] == fileName) {
                alreadyIncludedThis = true;
                break;
            }
        }

        getFileNamePosition(allFileNames, fileName);

        if(recursionLevel > 20) {
            if(errorStream) {
                (*errorStream) << "Too many levels of #include recursion in shader: " << fileName << endl;
            }
            return;
        }

        unsigned int lineNumber = 0;

        // Add header
        if(inputConstants) {
            for(unsigned int i = 0; i < (*inputConstants).size(); i++) {
                currentFileOutput.push_back(string("#define ") + (*inputConstants)[i]);
            }
        }

        // Correct the line number.
        currentFileOutput.push_back(

            getFileLineNumber(
                allFileNames, fileName, 0));

        while(lineNumber < input.size()) {

            int c = 0;

            // Read to the first non-whitespace thing in a line.
            while(input[lineNumber][c] && isWhiteSpace(input[lineNumber][c])) {
                c++;
            }

            // TODO: Extend this to understand "#pragma once" or at
            // the very least #ifdefs so we can make #include guards.

            if(input[lineNumber][c] == '#') {

                // Skip the '#'
                c++;

                // Tokenize it.
                std::vector<std::string> preProcTokens;
                stringTokenize(input[lineNumber].c_str() + c, " ", preProcTokens, false);

                if(preProcTokens.size()) {

                    if(preProcTokens[0] == "include") {

                        if(preProcTokens.size() > 1) {

                            string includeFileName = preProcTokens[1];

                            // Knock off quotes or brackets around the
                            // name. They don't really matter here.
                            if(includeFileName[0] == '"' || includeFileName[0] == '<') {

                                // Chop off the end.
                                includeFileName[includeFileName.size() - 1] = 0;

                                // Chop off the start.
                                includeFileName = includeFileName.c_str() + 1;

                                // Note, if this code changes, be
                                // aware that C++ strings will happily
                                // store the \0 in the string, and
                                // screw everything up later.
                            }

                            // Make sure we work with
                            // directory-relative stuff.
                            string dirName = FileSystem::getParentName(fileName);
                            string fullIncludeFilePath = includeFileName;
                            if(dirName.size()) {
                                fullIncludeFilePath = dirName + "/" + fullIncludeFilePath;
                            }

                            // Load the included file.
                            std::string code;

                            // Try to load from the current directory
                            // first.
                            code = FileSystem::loadFileString(fullIncludeFilePath);

                            // If that failed, start going through the
                            // include paths.
                            if(!code.size() && includePaths) {
                                for(unsigned int i = 0; i < includePaths->size(); i++) {

                                    // Correct any trailing forward
                                    // slash on the path so we can add
                                    // it back in. (Derp.)
                                    string path = (*includePaths)[i];
                                    if(path.size() && path[path.size() - 1] == '/') {
                                        path.substr(0, path.size() - 1);
                                    }

                                    // Try to load the file and break
                                    // out of the loop if we succeed.
                                    code = FileSystem::loadFileString(path + "/" + includeFileName);
                                    if(code.size()) break;
                                }
                            }

                            if(code.size()) {

                                // Process another file's contents
                                // straight into our own output list.
                                vector<string> inputLines;
                                stringTokenize(code, "\n", inputLines, true);
                                simplePreprocess(
                                    allFileNames,
                                    includeFileName,
                                    inputLines,
                                    currentFileOutput, NULL,
                                    errorStream,
                                    recursionLevel + 1,
                                    includePaths);

                                // Get back to the correct line
                                // number.

                                // // FIXME: Apparently this doesn't work
                                // // with shaders on Intel's
                                // // drivers. Works on nVidia, need to
                                // // test ATI.
                                // ostringstream str;
                                // str << "#line " << lineNumber;
                                // currentFileOutput.push_back(str.str());

                                currentFileOutput.push_back(
                                    getFileLineNumber(
                                        allFileNames, fileName, lineNumber));

                            } else {

                                // Error: Bad #include
                                if(errorStream) {
                                    (*errorStream) << "Couldn't open " << includeFileName.c_str() << "." << endl;
                                }
                            }

                        } else {
                            // Error: Bad #include
                        }

                    } else if(preProcTokens[0] == "pragma") {

                        bool pragmaHandled = false;

                        if(preProcTokens.size() > 1) {

                            if(preProcTokens[1] == "once") {

                                // pragma once means bail out before
                                // we end up adding the current output
                                // buffer to the main output buffer if
                                // we've already included this file.
                                if(alreadyIncludedThis) {
                                    return;
                                }
                                pragmaHandled = true;
                            }

                        }

                        if(!pragmaHandled) {
                            // Whatever the next preprocessor is might
                            // recognize this.
                            currentFileOutput.push_back(input[lineNumber]);
                        }

                    } else {
                        // If it's a directive we don't recognize
                        // here, it's probably something that GLSL
                        // already handles, so just pass it through.
                        currentFileOutput.push_back(input[lineNumber]);
                    }

                } else {
                    // Error: Had just a '#' on the line.
                }

            } else {

                // Normal line of code.
                currentFileOutput.push_back(input[lineNumber]);

            }

            lineNumber++;
        }

        // Add this file's stuff onto the current output.
        output.insert(output.end(), currentFileOutput.begin(), currentFileOutput.end());

    }
}

