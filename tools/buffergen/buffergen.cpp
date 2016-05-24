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

// This is just a simple program to turn some input file into a header
// file so we can include it right into the executable.

#include <iostream>
#include <iomanip>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

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

int main(int argc, char *argv[])
{
    // Parse params. We only have a help parameter here, because this
    // thing is so simple.
    std::vector<std::string> paramNames = {};
    std::vector<ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);
    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            cout << "Usage: " << argv[0] << " <filename> [variable]" << endl;
            cout << endl;
            cout << "ExpiredPopsicle's static buffer header generator 1.0" << endl;
            cout << "Useful for when you really want to embed a whole damn" << endl;
            cout << "file into your source code." << endl;
            cout << endl;
            cout << "Options:" << endl;
            cout << endl;
            cout << "  --help            You're sitting in it." << endl;
            cout << endl;
            cout << "Report bugs to expiredpopsicle@gmail.com" << endl;
            exit(0);
        }
    }

    if(argc < 2) {
        cerr << "Specify a file name and possibly a variable name!" << endl;
        return 1;
    }

    int fileLen = 0;
    char *fileData = FileSystem::loadFile(argv[1], &fileLen);

    if(!fileData) {
        cerr << "Failed to open that file." << endl;
        return 1;
    }

    string variableName;
    if(argc >= 3) {
        variableName = argv[2];
    } else {
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

    for(unsigned int i = 0; i < (unsigned int)fileLen + 1; i++) {
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



