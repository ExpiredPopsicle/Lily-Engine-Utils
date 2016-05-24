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

#pragma once

#include <map>
#include <string>
#include <fstream>
#include <vector>

#ifdef OS_ANDROID
struct AAsset;
#endif

// Simple file archive system.

namespace ExPop {

    namespace FileSystem {

        /// File archive (like pk3 files, but slightly less
        /// cool). Most FileSystem functions can be told to use one or
        /// more archives as fallbacks in case they don't exist on the
        /// real filesystem.
        class Archive {
        public:

            /// Default constructor creates a not-really-working
            /// archive. Don't use it unless it's part of the
            /// implementation for a sub-class.
            Archive(void);

            /// Constructor. Give it a file name and writable flag.
            /// Writable flag should be reserved for archive creation.
            Archive(const std::string &fileName, bool writeMode = false);

            virtual ~Archive(void);

            /// Add a file to the archive. Archive must have been created
            /// in write mode or this will fail. (assert fail)
            virtual void addFile(const std::string &fileName, const char *data, int length);

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
