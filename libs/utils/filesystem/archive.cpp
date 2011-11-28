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

#include <malloc.h>
#include <cassert>
#include <iostream>
#include <cstring>
using namespace std;

#include "filesystem.h"
#include "archive.h"

#ifdef OS_ANDROID
#include <android/asset_manager.h>
#include "../../hacks/android.h"
#endif

namespace ExPop {

    namespace FileSystem {

        struct FileHeader {
            // Name of the file.
            char name[512];

            // Length of the file (little endian, 32 bits).
            // Does not include the size of this header.
            unsigned int length;
        };

        Archive::Archive(void) {

            // No archive, or something with overridden functionality.
            failed = true;
            this->writeMode = false;

#ifdef OS_ANDROID
            archiveFileAsset = NULL;
#endif
        }

        Archive::Archive(const std::string &fileName, bool writeMode) {

            failed = false;

#ifdef OS_ANDROID

            // I don't think we're going to be porting tools to Android,
            // so writeMode is entirely invalid here.
            assert(!writeMode);

            archiveFileAsset = AAssetManager_open(
                androidState->activity->assetManager,
                fileName.c_str(),
                AASSET_MODE_RANDOM);

            this->writeMode = false;

#else

            if(writeMode) {
                archiveFile.open(fileName.c_str(), ios::out | ios::binary);
            } else {
                archiveFile.open(fileName.c_str(), ios::in | ios::binary);
            }
            this->writeMode = writeMode;

#endif

            myFileName = fileName;

#ifdef OS_ANDROID

            if(!archiveFileAsset) {
                failed = true;
            }

#else

            if(archiveFile.fail()) {
                failed = true;
                return;
            }

#endif

            // Scan through the whole file and build up the TOC.
            if(!writeMode) {
                rebuildTableOfContents();
                closeArchiveFile();
            }
        }

        Archive::~Archive(void) {
            closeArchiveFile();
        }

        void Archive::openArchiveFile(void) {

#ifdef OS_ANDROID

            if(!archiveFileAsset) {

                archiveFileAsset = AAssetManager_open(
                    androidState->activity->assetManager,
                    myFileName.c_str(),
                    AASSET_MODE_RANDOM);
            }

#else

            if(!archiveFile.is_open()) {
                archiveFile.open(myFileName.c_str(), ios::binary | (writeMode ? ios::out : ios::in) );
            }

#endif

        }
        void Archive::closeArchiveFile(void) {

#ifdef OS_ANDROID

            if(archiveFileAsset) {
                AAsset_close(archiveFileAsset);
                archiveFileAsset = NULL;
            }

#else

            if(archiveFile.is_open()) {
                archiveFile.close();
            }

#endif

        }

        void Archive::addFile(const std::string &fileName, const char *data, int length) {

            if(failed) return;

#ifdef OS_ANDROID

            // No writing on Android!
            assert(0);

#else

            assert(archiveFile.is_open());
            assert(writeMode);

            // // First, see if the file is already in the archive.
            // if(tableOfContents[fileName]) {
            //     // If it is, remove it.
            //     deleteFile(fileName);
            // }

            // Go to the end of the archive.
            archiveFile.seekp(0, ios_base::end);
            unsigned int currentEnd = archiveFile.tellp();

            // Make the filename consistent.
            string fixedFileName = fixFileName(fileName);

            // Write the header.
            FileHeader *header = (FileHeader*)calloc(1, sizeof(FileHeader));
            assert(header);
            strncpy(header->name, fixedFileName.c_str(), 511);
            header->length = length;
            archiveFile.write((char*)header, sizeof(FileHeader));
            free(header);

            // Write the data.
            archiveFile.write(data, length);

            // Update TOC.
            tableOfContents[fileName] = currentEnd;

#endif

        }

        char *Archive::loadFile(const std::string &fileName, int *length, bool addNullTerminator) {

            if(failed) return NULL;
            assert(!writeMode);
            openArchiveFile();

            string fixedFileName = fixFileName(fileName);

            // Get our offset of the header in the archive.
            unsigned int offset;
            std::map<std::string, unsigned int>::iterator iter =
                tableOfContents.find(fixedFileName);

            if(iter == tableOfContents.end()) {
                return NULL; // Not in this archive!
            }

            offset = iter->second;

            // Go to the offset in the archive and read in the header.
            FileHeader header;

#ifdef OS_ANDROID
            AAsset_seek(archiveFileAsset, offset, SEEK_SET);
            AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));
#else
            archiveFile.seekg(offset, ios_base::beg);
            archiveFile.read((char*)&header, sizeof(FileHeader));
#endif

            // Make a place to put the data.
            char *data = new char[header.length + (addNullTerminator ? 1 : 0)];
            if(!data) return NULL; // Too big?

            // TODO: We should probably put an upper limit on the data size
            //   regardless of what we read from the archive. Allocating too
            //   much memory can be really bad.

            // Read in the data.

#ifdef OS_ANDROID
            AAsset_read(archiveFileAsset, data, header.length);
#else
            archiveFile.read(data, header.length);
#endif

            *length = header.length;

            if(addNullTerminator) {
                data[*length] = 0;
                (*length)++;
            }

            closeArchiveFile();
            return data;
        }

        char *Archive::loadFileHead(const std::string &fileName, int headerLength) {

            // Same as above, but only reads the first headerLength bytes.

            if(failed) return NULL;
            assert(!writeMode);
            openArchiveFile();

            string fixedFileName = fixFileName(fileName);

            // Get our offset of the header in the archive.
            unsigned int offset;
            std::map<std::string, unsigned int>::iterator iter =
                tableOfContents.find(fixedFileName);

            if(iter == tableOfContents.end()) {
                return NULL; // Not in this archive!
            }

            offset = iter->second;

            // Go to the offset in the archive and read in the header.
            FileHeader header;

#ifdef OS_ANDROID
            AAsset_seek(archiveFileAsset, offset, SEEK_SET);
            AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));
#else
            archiveFile.seekg(offset, ios_base::beg);
            archiveFile.read((char*)&header, sizeof(FileHeader));
#endif

            // Make a place to put the data.
            char *data = new char[headerLength];
            if(!data) return NULL; // Too big?

            memset(data, 0, headerLength);

            // Read in the data.

#ifdef OS_ANDROID
            AAsset_read(archiveFileAsset, data, (int)header.length < headerLength ? header.length : headerLength);
#else
            archiveFile.read(data, (int)header.length < headerLength ? header.length : headerLength);
#endif

            closeArchiveFile();
            return data;
        }

        void Archive::rebuildTableOfContents(void) {

            tableOfContents.clear();
            tableOfDirectories.clear();

            assert(!writeMode);
            openArchiveFile();

            if(failed) return;

            bool foundEof = false;

#ifdef OS_ANDROID

            AAsset_seek(archiveFileAsset, 0, SEEK_SET);
            int currentPos = 0;

#else

            archiveFile.seekg(0, ios_base::beg);

            // Clear whatever last EOF or error might be sitting around.
            archiveFile.clear();

#endif

            std::map<std::string, std::map<std::string, bool> > directoriesInDirectories;

            while(!foundEof) {

                FileHeader header;

#ifdef OS_ANDROID

                AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));

                if(!AAsset_getRemainingLength(archiveFileAsset)) {
                    break;
                }
#else

                int currentPos = archiveFile.tellg();

                archiveFile.read((char*)&header, sizeof(FileHeader));

                if(archiveFile.fail()) {
                    archiveFile.clear();
                    break;
                }
#endif

                tableOfContents[header.name] = currentPos;

                currentPos += header.length + sizeof(FileHeader);

                // Add it to the appropriate directory entry.
                string dirName = getParentName(header.name);
                string baseName = header.name + dirName.size();

                // Get rid of that last '/'.
                if(baseName.size()) {
                    baseName = baseName.c_str() + 1;
                }

                directoryContents[dirName].push_back(baseName);

                while(dirName.size()) {
                    string parentName = getParentName(dirName);
                    directoriesInDirectories[parentName][getBaseName(dirName)] = true;
                    dirName = parentName;
                }

                // Next file.

#ifdef OS_ANDROID
                AAsset_seek(archiveFileAsset, currentPos, SEEK_SET);
                foundEof = !AAsset_getRemainingLength(archiveFileAsset);
#else
                archiveFile.seekg(header.length, ios_base::cur);
                foundEof = archiveFile.eof();
#endif

            }

            // Add all the directories found to the directory directories.

            // These iterator loops look ugly as hell. Can I use the 'auto'
            // keyword yet? Ughh...
            for(std::map<std::string, std::map<std::string, bool> >::iterator outerIterator = directoriesInDirectories.begin(); outerIterator != directoriesInDirectories.end(); outerIterator++) {
                for(std::map<std::string, bool>::iterator innerIterator = (*outerIterator).second.begin(); innerIterator != (*outerIterator).second.end(); innerIterator++) {

                    string outerDir = (*outerIterator).first;
                    string innerDir = (*innerIterator).first;

                    //cout << "Dir in Dir: " << outerDir << " / " << innerDir << endl;
                    subdirectories[outerDir].push_back(innerDir);

                    if(outerDir.size()) outerDir = outerDir + "/";
                    tableOfDirectories[outerDir + innerDir] = true;

                }
            }

            closeArchiveFile();
        }

        bool Archive::getFileExists(const std::string &fileName) {
            if(failed) return false;

            string fixedFileName = fixFileName(fileName);

            std::map<std::string, unsigned int>::iterator iter =
                tableOfContents.find(fixedFileName);

            return !(iter == tableOfContents.end());
        }

        bool Archive::getDirExists(const std::string &dirName) {
            if(failed) return false;

            string fixedDirName = fixFileName(dirName);

            std::map<std::string, bool>::iterator iter =
                tableOfDirectories.find(fixedDirName);

            return !(iter == tableOfDirectories.end());
        }

        unsigned int Archive::getFileSize(const std::string &fileName) {
            if(failed) return 0;
            assert(!writeMode);
            openArchiveFile();

            string fixedFileName = fixFileName(fileName);

            if(!getFileExists(fixedFileName)) {
                return 0;
            }

            FileHeader header;

#ifdef OS_ANDROID
            AAsset_seek(archiveFileAsset, tableOfContents[fileName], SEEK_SET);
            AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));
#else
            archiveFile.seekg(tableOfContents[fileName], ios_base::beg);
            archiveFile.read((char*)&header, sizeof(FileHeader));
#endif

            closeArchiveFile();
            return header.length;

        }

        void Archive::getFileList(std::vector<std::string> &fileList) {
            std::map<std::string, unsigned int>::iterator iter;

            for(iter = tableOfContents.begin(); iter != tableOfContents.end(); iter++) {
                fileList.push_back(iter->first);
            }
        }

        void Archive::getFileListForDir(const std::string &dirName, std::vector<std::string> &fileList) {

            // In case anyone decided to be cute and use "." as the
            // directory name...
            string fixedDirName = fixFileName(dirName);

            fileList.insert(fileList.end(), subdirectories[fixedDirName].begin(), subdirectories[fixedDirName].end());
            fileList.insert(fileList.end(), directoryContents[fixedDirName].begin(), directoryContents[fixedDirName].end());
        }

        const std::string &Archive::getMyFileName(void) {
            // Should we return the fixed version of this?
            return myFileName;
        }

        bool Archive::getFailed(void) {
            return failed;
        }

    }
}
