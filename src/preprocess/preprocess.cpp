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

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#include "malstring.h"
#include "filesystem.h"
#include "preprocess.h"

namespace ExPop {

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
                            char *code = NULL;
                            int codeLength = 0;

                            // Try to load from the current directory
                            // first.
                            code = FileSystem::loadFile(fullIncludeFilePath, &codeLength, true);

                            // If that failed, start going through the
                            // include paths.
                            if(!code && includePaths) {
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
                                    code = FileSystem::loadFile(path + "/" + includeFileName, &codeLength, true);
                                    if(code) break;
                                }
                            }

                            if(code) {

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

                                delete[] code;

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


