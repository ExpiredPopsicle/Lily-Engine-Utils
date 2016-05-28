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

// This is just a simple program to turn some input file into a header
// file so we can include it right into the executable.

#include <iostream>
#include <iomanip>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

// buffergen depends on itself to generate the --help text in
// usageText.h. We'll build it first with NO_HELP_ALLOWED so that we
// can use it to make the --help text. Then we'll build the final
// version.
#if !NO_HELP_ALLOWED
#include "usagetext.h"
#else
const unsigned int usageText_len = 13;
const char usageText[] = "No help here!";
#endif

std::string sanitizeName(const std::string &in)
{
    string out;
    out.reserve(in.size());
    for(unsigned int i = 0; i < in.size(); i++) {
        if((in[i] >= 'a' && in[i] <= 'z') ||
           (in[i] >= 'A' && in[i] <= 'Z') ||
           (in[i] >= '0' && in[i] <= '9') ||
           in[i] == '_') {
            out = out + in[i];
        } else {
            out = out + "_";
        }
    }
    return out;
}

void showHelp(const char *argv0, bool error)
{
    std::ostream *out = error ? &cerr : &cout;
    (*out) << stringReplace<char>("$0", argv0, std::string(usageText, usageText_len)) << endl;
}

int main(int argc, char *argv[])
{
    // Parse params. We only have a help parameter here, because this
    // thing is so simple.
    std::vector<std::string> paramNames = {};
    std::vector<ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);

    bool addNull = false;
    std::string filename = "";
    std::string variableName = "";

    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            showHelp(argv[0], false);
            return 0;
        } else if(params[i].name == "nonull") {
            addNull = true;
        } else if(params[i].name == "") {
            if(!filename.size()) {
                filename = params[i].value;
            } else if(!variableName.size()) {
                variableName = params[i].value;
            } else {
                cerr << "Too many parameters starting at: " << params[i].value << endl;
            }
        }
    }

    if(!filename.size()) {
        showHelp(argv[0], true);
        return 1;
    }

    int fileLen = 0;
    char *fileData = FileSystem::loadFile(filename, &fileLen);

    if(!fileData) {
        cerr << "Failed to open " << filename << endl;
        return 1;
    }

    // No variable name? Make one up based on the filename.
    if(!variableName.size()) {
        string base = ExPop::FileSystem::getBaseName(argv[1]);
        string baseNoExt;
        string junk;
        stringSplit(base, ".", baseNoExt, junk, true);
        variableName = sanitizeName(baseNoExt);
    }

    cout << "const unsigned int " << variableName << "_len = " << fileLen << ";" << endl;
    cout << "const char " << variableName << "[] = {" << endl;

    // Note that this loop goes to one past the end so we can add a
    // null terminating character and use it as a string later. The
    // _len variable will show the actual size of the buffer without
    // the extra terminator, however.

    for(unsigned int i = 0; i < (unsigned int)fileLen + (addNull ? 1 : 0); i++) {
        if(!(i % 20)) {
            if(i) cout << endl;

            // Three spaces here because numbers have a leading one.
            cout << "   ";
        }

        unsigned char outChar = 0;

        if(i < (unsigned int)fileLen)
            outChar = fileData[i];

        cout << " 0x" << setw(2) << setfill('0') << hex << (unsigned int)outChar << ",";
    }

    cout << endl;
    cout << "};" << endl;

    return 0;

}



