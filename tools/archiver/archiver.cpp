#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

#include <lilyengine/archive.h>
#include <lilyengine/filesystem.h>
using namespace ExPop;

int main(int argc, char *argv[]) {

    if(argc > 2 && !strcmp(argv[1], "-c")) {

        FileSystem::Archive outFile(argv[2], true);
        if(outFile.getFailed()) {
            cerr << "Could not open file for writing: " << argv[2] << endl;
            return 1;
        }

        for(int i = 3; i < argc; i++) {

            int len = 0;

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
        cout << "Usages: " << endl;
        cout << "  " << argv[0] << " -c <archive filename> [files to add]     - Create an archive." << endl;
        cout << "  " << argv[0] << " -e <archive filename> [files to extract] - Extract files from an archive." << endl;
        cout << "  " << argv[0] << " -l <archive filename>                    - List files in an archive." << endl;
        cout << "  " << argv[0] << " -d <archive filename> [files to dump]    - Dump specified files to stdout." << endl;
        return 1;
    }

    return 1;
}
