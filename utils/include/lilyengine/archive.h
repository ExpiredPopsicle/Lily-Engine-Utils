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

// Simple file archive system. This is intended to integrate
// seamlessly with the FileSystem module to provide a transparent
// overlay to the real filesystem.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <map>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstring>

#ifdef OS_ANDROID
#include <android/asset_manager.h>
struct AAsset;
#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace FileSystem
    {
        /// File archive (like pk3 files, but slightly less
        /// cool). Most FileSystem functions can be told to use one or
        /// more archives as fallbacks in case they don't exist on the
        /// real filesystem.
        class Archive
        {
        public:

            /// Default constructor creates a not-really-working
            /// archive. Don't use it unless it's part of the
            /// implementation for a sub-class.
            Archive(void);

            /// Constructor. Give it a file name and writable flag.
            /// Writable flag should be reserved for archive creation.
            Archive(const std::string &fileName, bool writeMode = false);

            virtual ~Archive(void);

            /// Add a file to the archive. Archive must have been
            /// created in write mode or this will fail. Returns false
            /// on failure, or true on success.
            virtual bool addFile(const std::string &fileName, const char *data, int length);

            /// Load a file from the archive. Length will be stored in
            /// length. Set addNullTerminator to true for non-binary
            /// data that should have a null terminator. Value stored
            /// in length will include this extra byte if
            /// specified. Data is owned by caller. Clean it up with
            /// delete[].
            virtual char *loadFile(const std::string &fileName, int *length, bool addNullTerminator = false);

            /// Reads a piece of a file instead of the whole thing.
            virtual char *loadFilePart(const std::string &fileName, int lengthToRead, int offsetFromStart = 0);

            /// Get a list of all files in the archive with full path
            /// names.
            void getFileList(std::vector<std::string> &fileList);

            /// Check if a file exists in the archive. Use full path
            /// from archive root. Does not return true for
            /// directories.
            bool getFileExists(const std::string &fileName);

            /// Check if a directory exists. Note that directories
            /// will not exist on their own inside an archive. There
            /// must be something in a directory for it to be
            /// stored. (Directory information is also built off the
            /// paths of normal files in those directories.)c
            bool getDirExists(const std::string &dirName);

            /// Get the size of a file. Does not include additional
            /// null terminator if one is needed.
            virtual unsigned int getFileSize(const std::string &fileName);

            /// Get list of files and directories in a given
            /// directory.
            void getFileListForDir(const std::string &dirName, std::vector<std::string> &fileList);

            /// Get the file name of this archive.
            const std::string &getMyFileName(void);

            /// Get error condition, which could be caused by any
            /// number of things, but all basically amount to nothing
            /// useful coming out of this archive.
            bool getFailed(void);

        protected:

            void closeArchiveFile(void);
            void openArchiveFile(void);

            // Maps filenames (strings) to byte offsets within the file (ints).
            std::map<std::string, unsigned int> tableOfContents;
            std::map<std::string, bool> tableOfDirectories;
            std::map<std::string, std::vector<std::string> > directoryContents;
            std::map<std::string, std::vector<std::string> > subdirectories;
            std::string myFileName;

          #ifdef OS_ANDROID
            AAsset *archiveFileAsset;
          #else
            std::fstream archiveFile;
          #endif

            bool writeMode;
            bool failed;
            virtual void rebuildTableOfContents(void);
        };

    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#include "filesystem.h"

namespace ExPop
{
    namespace FileSystem
    {
        struct FileHeader
        {
            // Name of the file.
            char name[512];

            // Length of the file (little endian, 32 bits).
            // Does not include the size of this header.
            unsigned int length;
        };

        inline Archive::Archive(void)
        {
            // No archive, or something with overridden functionality.
            failed = true;
            this->writeMode = false;

          #ifdef OS_ANDROID
            archiveFileAsset = NULL;
          #endif
        }

        inline Archive::Archive(const std::string &fileName, bool writeMode)
        {
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
                archiveFile.open(fileName.c_str(), std::ios::out | std::ios::binary);
            } else {
                archiveFile.open(fileName.c_str(), std::ios::in | std::ios::binary);
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

        inline Archive::~Archive(void)
        {
            closeArchiveFile();
        }

        inline void Archive::openArchiveFile(void)
        {
          #ifdef OS_ANDROID

            if(!archiveFileAsset) {

                archiveFileAsset = AAssetManager_open(
                    androidState->activity->assetManager,
                    myFileName.c_str(),
                    AASSET_MODE_RANDOM);
            }

          #else

            if(!archiveFile.is_open()) {
                archiveFile.open(myFileName.c_str(), std::ios::binary | (writeMode ? std::ios::out : std::ios::in) );
            }

          #endif
        }

        inline void Archive::closeArchiveFile(void)
        {
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

        inline bool Archive::addFile(const std::string &fileName, const char *data, int length)
        {
            if(failed) {
                return false;
            }

          #ifdef OS_ANDROID

            // No writing on Android!
            assert(0);

          #else

            assert(archiveFile.is_open());

            if(!writeMode) {
                return false;
            }

            // First, see if the file is already in the archive.
            if(tableOfContents.find(fileName) != tableOfContents.end()) {
                return false;
            }

            // Go to the end of the archive.
            archiveFile.seekp(0, std::ios_base::end);
            unsigned int currentEnd = archiveFile.tellp();

            // Make the filename consistent.
            std::string fixedFileName = fixFileName(fileName);

            // Write the header.
            FileHeader *header = (FileHeader*)calloc(1, sizeof(FileHeader));
            assert(header);
            strncpy(header->name, fixedFileName.c_str(), 511);
            header->length = length;
            archiveFile.write((char*)header, sizeof(FileHeader));
            free(header);

            if(archiveFile.rdstate() & std::ifstream::failbit) {
                return false;
            }

            // Write the data.
            archiveFile.write(data, length);

            if(archiveFile.rdstate() & std::ifstream::failbit) {
                return false;
            }

            // Update TOC.
            tableOfContents[fileName] = currentEnd;

          #endif

            return true;
        }

        inline char *Archive::loadFile(const std::string &fileName, int *length, bool addNullTerminator)
        {
            if(failed) return NULL;
            assert(!writeMode);
            openArchiveFile();

            std::string fixedFileName = fixFileName(fileName);

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
            archiveFile.seekg(offset, std::ios_base::beg);
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

        inline char *Archive::loadFilePart(const std::string &fileName, int lengthToRead, int offsetFromStart)
        {
            if(failed) return NULL;
            assert(!writeMode);
            openArchiveFile();

            std::string fixedFileName = fixFileName(fileName);

            // Get our offset of the header in the archive.
            unsigned int offset;
            std::map<std::string, unsigned int>::iterator iter =
                tableOfContents.find(fixedFileName);

            if(iter == tableOfContents.end()) {
                return NULL; // Not in this archive!
            }

            offset = iter->second;
            offset += offsetFromStart;

            // Go to the offset in the archive and read in the header.
            FileHeader header;

          #ifdef OS_ANDROID
            AAsset_seek(archiveFileAsset, offset, SEEK_SET);
            AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));
          #else
            archiveFile.seekg(offset, std::ios_base::beg);
            archiveFile.read((char*)&header, sizeof(FileHeader));
          #endif

            // Make a place to put the data.
            char *data = new char[lengthToRead];
            if(!data) return NULL; // Too big?

            memset(data, 0, lengthToRead);

            // Read in the data.

          #ifdef OS_ANDROID
            AAsset_read(archiveFileAsset, data, (int)header.length < lengthToRead ? header.length : lengthToRead);
          #else
            archiveFile.read(data, (int)header.length < lengthToRead ? header.length : lengthToRead);
          #endif

            closeArchiveFile();
            return data;
        }

        inline void Archive::rebuildTableOfContents(void)
        {
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
            archiveFile.seekg(0, std::ios_base::beg);
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

                // Just for safety, add in a NULL terminator to the
                // last position of the name string, otherwise bad
                // archives could have us read off the end of the
                // filename into whatever.
                header.name[511] = 0;

                tableOfContents[header.name] = currentPos;

                currentPos += header.length + sizeof(FileHeader);

                // Add it to the appropriate directory entry.
                std::string dirName = getParentName(header.name);
                std::string baseName = header.name + dirName.size();

                // Get rid of that last '/'.
                if(baseName.size()) {
                    baseName = baseName.c_str() + 1;
                }

                directoryContents[dirName].push_back(baseName);

                while(dirName.size()) {
                    std::string parentName = getParentName(dirName);
                    directoriesInDirectories[parentName][getBaseName(dirName)] = true;
                    dirName = parentName;
                }

                // Next file.

              #ifdef OS_ANDROID
                AAsset_seek(archiveFileAsset, currentPos, SEEK_SET);
                foundEof = !AAsset_getRemainingLength(archiveFileAsset);
              #else
                archiveFile.seekg(header.length, std::ios_base::cur);
                foundEof = archiveFile.eof();
              #endif

            }

            // Add all the directories found to the directory directories.

            // These iterator loops look ugly as hell. Can I use the 'auto'
            // keyword yet? Ughh...
            for(std::map<std::string, std::map<std::string, bool> >::iterator outerIterator = directoriesInDirectories.begin(); outerIterator != directoriesInDirectories.end(); outerIterator++) {
                for(std::map<std::string, bool>::iterator innerIterator = (*outerIterator).second.begin(); innerIterator != (*outerIterator).second.end(); innerIterator++) {

                    std::string outerDir = (*outerIterator).first;
                    std::string innerDir = (*innerIterator).first;

                    //cout << "Dir in Dir: " << outerDir << " / " << innerDir << endl;
                    subdirectories[outerDir].push_back(innerDir);

                    if(outerDir.size()) outerDir = outerDir + "/";
                    tableOfDirectories[outerDir + innerDir] = true;

                }
            }

            closeArchiveFile();
        }

        inline bool Archive::getFileExists(const std::string &fileName)
        {
            if(failed) return false;

            std::string fixedFileName = fixFileName(fileName);

            std::map<std::string, unsigned int>::iterator iter =
                tableOfContents.find(fixedFileName);

            return !(iter == tableOfContents.end());
        }

        inline bool Archive::getDirExists(const std::string &dirName)
        {
            if(failed) return false;

            std::string fixedDirName = fixFileName(dirName);

            std::map<std::string, bool>::iterator iter =
                tableOfDirectories.find(fixedDirName);

            return !(iter == tableOfDirectories.end());
        }

        inline unsigned int Archive::getFileSize(const std::string &fileName)
        {
            if(failed) return 0;
            assert(!writeMode);
            openArchiveFile();

            std::string fixedFileName = fixFileName(fileName);

            if(!getFileExists(fixedFileName)) {
                return 0;
            }

            FileHeader header;

          #ifdef OS_ANDROID
            AAsset_seek(archiveFileAsset, tableOfContents[fileName], SEEK_SET);
            AAsset_read(archiveFileAsset, (char*)&header, sizeof(FileHeader));
          #else
            archiveFile.seekg(tableOfContents[fileName], std::ios_base::beg);
            archiveFile.read((char*)&header, sizeof(FileHeader));
          #endif

            closeArchiveFile();
            return header.length;

        }

        inline void Archive::getFileList(std::vector<std::string> &fileList)
        {
            std::map<std::string, unsigned int>::iterator iter;

            for(iter = tableOfContents.begin(); iter != tableOfContents.end(); iter++) {
                fileList.push_back(iter->first);
            }
        }

        inline void Archive::getFileListForDir(const std::string &dirName, std::vector<std::string> &fileList)
        {
            // In case anyone decided to be cute and use "." as the
            // directory name...
            std::string fixedDirName = fixFileName(dirName);

            fileList.insert(fileList.end(), subdirectories[fixedDirName].begin(), subdirectories[fixedDirName].end());
            fileList.insert(fileList.end(), directoryContents[fixedDirName].begin(), directoryContents[fixedDirName].end());
        }

        inline const std::string &Archive::getMyFileName(void)
        {
            // Should we return the fixed version of this?
            return myFileName;
        }

        inline bool Archive::getFailed(void)
        {
            return failed;
        }
    }
}

