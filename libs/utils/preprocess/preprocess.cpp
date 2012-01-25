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

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "../string/malstring.h"
#include "../filesystem/filesystem.h"
#include "preprocess.h"

namespace ExPop {

    // static unsigned int getFileNamePosition(
    //     vector<string> &allFileNames,
    //     const string &name) {

    //     for(unsigned int i = 0; i < allFileNames.size(); i++) {
    //         if(allFileNames[i] == name) {
    //             return i;
    //         }
    //     }

    //     allFileNames.push_back(name);
    //     return allFileNames.size() - 1;
    // }

    static string getFileLineNumber(
        vector<string> &allFileNames,
        const string &name,
        unsigned int lineNumber) {

        // This is disabled because it's borked on Intel shader
        // compiles.

        // ostringstream str;
        // str <<
        //     "//#line " << lineNumber <<
        //     " " << getFileNamePosition(allFileNames, name) <<
        //     " // " << name;
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
        int recursionLevel) {

        if(!errorStream) errorStream = &cout;

        if(recursionLevel > 20) {
            (*errorStream) << "Too many levels of #include recursion in shader: " << fileName << endl;
            return;
        }

        unsigned int lineNumber = 0;

        // Add header
        if(inputConstants) {
            for(unsigned int i = 0; i < (*inputConstants).size(); i++) {
                output.push_back(string("#define ") + (*inputConstants)[i]);
            }
        }

        // Correct the line number.
        output.push_back(
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

                            // Load the included file.
                            char *code = NULL;
                            int codeLength = 0;
                            code = FileSystem::loadFile(includeFileName, &codeLength, true);

                            if(code) {

                                // Process another file's contents
                                // straight into our own output list.
                                vector<string> inputLines;
                                stringTokenize(code, "\n", inputLines, true);
                                simplePreprocess(
                                    allFileNames,
                                    includeFileName,
                                    inputLines,
                                    output, NULL,
                                    errorStream,
                                    recursionLevel + 1);

                                // Get back to the correct line
                                // number.

                                // // FIXME: Apparently this doesn't work
                                // // with shaders on Intel's
                                // // drivers. Works on nVidia, need to
                                // // test ATI.
                                // ostringstream str;
                                // str << "#line " << lineNumber;
                                // output.push_back(str.str());

                                output.push_back(
                                    getFileLineNumber(
                                        allFileNames, fileName, lineNumber));

                            } else {
                                // Error: Bad #include
                                (*errorStream) << "Couldn't open " << includeFileName.c_str() << "." << endl;
                            }

                            delete[] code;

                        } else {
                            // Error: Bad #include
                        }

                    } else {
                        // If it's a directive we don't recognize
                        // here, it's probably something that GLSL
                        // already handles, so just pass it through.
                        output.push_back(input[lineNumber]);
                    }

                } else {
                    // Error: Had just a '#' on the line.
                }

            } else {

                // Normal line of code.
                output.push_back(input[lineNumber]);

            }

            lineNumber++;
        }
    }

}


