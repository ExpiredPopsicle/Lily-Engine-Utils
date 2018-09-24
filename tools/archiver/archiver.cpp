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

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

#include <lilyengine/archive.h>
#include <lilyengine/filesystem.h>
#include <lilyengine/malstring.h>
using namespace ExPop;

#include "usagetext.h"

int main(int argc, char *argv[])
{
    // This was all written before the ExPop::parseCommandLine()
    // system existed. There's not really any good reason to change
    // it.

    if(argc > 2 && !strcmp(argv[1], "-c")) {

        FileSystem::Archive outFile(argv[2], true);
        if(outFile.getFailed()) {
            cerr << "Could not open file for writing: " << argv[2] << endl;
            return 1;
        }

        for(int i = 3; i < argc; i++) {

            int64_t len = 0;

            // Skip directories internally because they're a pain to
            // screen out on the command line.
            if(FileSystem::isDir(argv[i])) continue;

            char *data = FileSystem::loadFile(argv[i], &len);
            if(data) {
                outFile.addFile(FileSystem::fixFileName(argv[i]), data, len);
            } else {
                cerr << "Error reading " << argv[i] << endl;
                return 1;
            }
        }

        return 0;

    } else if(argc > 2 && !strcmp(argv[1], "-l")) {

        FileSystem::Archive inFile(argv[2], false);
        if(inFile.getFailed()) {
            cerr << "Could not open file for reading: " << argv[2] << endl;
            return 1;
        }

        vector<string> fileList;
        inFile.getFileList(fileList);

        for(unsigned int i = 0; i < fileList.size(); i++) {
            cout << fileList[i] << endl;
        }

    } else if(argc > 2 && !strcmp(argv[1], "-e")) {

        FileSystem::Archive inFile(argv[2], false);
        if(inFile.getFailed()) {
            cerr << "Could not open file for reading: " << argv[2] << endl;
            return 1;
        }

        for(int i = 3; i < argc; i++) {
            int len = 0;
            char *data = inFile.loadFile(argv[i], &len);
            if(data) {
                cout << argv[i] << endl;
                if(FileSystem::saveFile(argv[i], data, len, true)) {
                    cerr << "Error when writing file: " << argv[i];
                    return 1;
                }
            } else {
                cerr << "Could not load file from archive: " << argv[i] << endl;
                return 1;
            }
        }

        return 0;

    } else if(argc > 2 && !strcmp(argv[1], "-d")) {

        FileSystem::Archive inFile(argv[2], false);
        if(inFile.getFailed()) {
            cerr << "Could not open file for reading: " << argv[2] << endl;
            return 1;
        }

        for(int i = 3; i < argc; i++) {
            int len = 0;
            char *data = inFile.loadFile(argv[i], &len);
            if(data) {
                cout << data;
            } else {
                cerr << "Could not load file from archive: " << argv[i] << endl;
                return 1;
            }
        }

        return 0;

    } else {

        std::string helpString = stringReplace<char>("$0", argv[0], std::string(usageText, usageText_len));

        if(argc > 1 && !strcmp(argv[1], "--help")) {
            // If they actually asked for --help, then that's not
            // really an error.
            cout << helpString << endl;
            return 0;
        }

        // If they got here, they didn't ask for --help, but had to
        // get it anyway, probably because they screwed it up.
        cerr << helpString << endl;
        return 1;
    }

    return 1;
}
