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

// Filesystem access module.

// TODO: Check this:
// http://www.reddit.com/r/gamedev/comments/s7y83/saving_configuration_files_savegame_data_etc_on/

// TODO: Get path to binary. windows: GetModuleFileName unix: ???

// TODO: Maybe a thing to get the data directory, as it's been
// configured.

// TODO: Symlink detection on Windows?

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>

#include <lilyengine/filesystem.h>
#include <lilyengine/malstring.h>
#include <lilyengine/thread.h>

#if !_WIN32
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
#include <direct.h>
#endif

#include <sys/stat.h>
#include <stdio.h>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace FileSystem
    {
        /// Get all the files and subdirectories in a directory.
        /// Returns true on success and false on failure.
        bool getAllFiles(const std::string &directory, std::vector<std::string> &names);

        /// Get all the subdirectories in a directory. Returns true if
        /// successful, false otherwise.
        bool getSubdirectories(const std::string &directory, std::vector<std::string> &names);

        /// Get all the non-directory files in a directory. Returns
        /// true if successful, false otherwise.
        bool getNondirectories(const std::string &directory, std::vector<std::string> &names);

        /// Determine if a file exists. Returns true if it exists.
        bool fileExists(const std::string &fileName, bool skipArchives = false);

        /// Get a file timestamp as the number of seconds since the
        /// Unix epoch. Doesn't work with stuff stored in archives
        /// because timestamps aren't stored in archives.
        unsigned int fileChangedTimeStamp(const std::string &fileName);

        /// Determine if a file is a directory. Returns true if it is
        /// a directory.
        bool isDir(const std::string &fileName, bool skipArchives = false);

        /// Determine if a file is a symbolic link. (Always returns false
        /// on Windows.)
        bool isSymLink(const std::string &fileName);

        enum {
            FILEFLAG_ARCHIVED    = 1,
            FILEFLAG_NONARCHIVED = 2,
        };

        /// Get flags for a file (archived, non-archived, etc).
        unsigned int getFileFlags(const std::string &fileName);

        /// Get the name of the parent directory to the path. Returns
        /// the name of the parent directory, or "" if it's at the
        /// highest level.
        std::string getParentName(const std::string &dirPath);

        /// Get the name of the file without the directory.
        std::string getBaseName(const std::string &path);

        /// Recursively make directories to create a given path.
        /// Returns true if successful, false otherwise.
        bool makePath(const std::string &dirPath);

        /// Renames (moves) a file. Returns true if successful, false
        /// otherwise. This will probably NOT work to transfer things
        /// from one filesystem to another.
        bool renameFile(const std::string &src, const std::string &dst);

        /// Deletes a file (including empty directories). Returns true
        /// if successful, false otherwise.
        bool deleteFile(const std::string &fileName);

        /// Deletes a file or directory and all files or directories
        /// inside. Returns true if successful, false otherwise. May
        /// partially complete (some files removed).
        bool recursiveDelete(const std::string &fileName);

        /// Copies a file. Returns true if successful, false
        /// otherwise.
        bool copyFile(const std::string &src, const std::string &dst);

        /// Copies an entire tree of files and directories. Return
        /// true if successful, false otherwise. May partially
        /// complete.
        bool recursiveCopy(const std::string &src, const std::string &dst);

        /// Get the size of a file in bytes.
        int getFileSize(const std::string &fileName);

        /// Load a file to a new buffer (caller takes ownership of the
        /// buffer). Buffer length is stored in length. Use
        /// addNullTerminator if you intend to use the result as a string.
        /// length will be changed to reflect the size of the returned data.
        char *loadFile(const std::string &fileName, int *length, bool addNullTerminator = false);

        /// Load a file and just return it as an std::string.
        std::string loadFileString(const std::string &fileName);

        /// Load a part of a file into a new buffer. Only do this if
        /// you already know the length you're dealing with. If it
        /// hits a file in an archive it may start reading into the
        /// next file's header and the next file.
        char *loadFilePart(const std::string &fileName, int lengthToRead, int offsetFromStart = 0);

        /// Saves a buffer to a file. Returns -1 on failure or 0 on
        /// success.
        int saveFile(const std::string &fileName, const char *data, int length, bool mkDirTree = false);

        /// Add an archive file to fall back on when loading a file from
        /// the real filesystem fails.
        void addArchiveForSearch(const std::string &archiveFileName);

        /// Remove an archive from the search list.
        void removeArchiveForSearch(const std::string &archiveFileName);

        /// Remove all archives from the search list.
        void clearSearchArchives(void);

        /// Make a filename consistent. Interprets ".." and "." directories
        /// and properly strips them out. Replaces backslashes with forward
        /// slashes.
        std::string fixFileName(const std::string &str);

        /// Filter a list of files by extension. The filtered list is ADDED
        /// to outputList. extension is a comma or space separated list of
        /// extensions to include. Don't include the '.' in the extensions.
        void filterFileListByType(
            const std::string &extension,
            const std::vector<std::string> &inputList,
            std::vector<std::string> &outputList);

        /// Get the current working directory. Note that this will NOT
        /// add a leading slash at the end unless it is the root
        /// directory on a Unix filesystem or the root directory on
        /// some Windows drive.
        std::string getCwd(void);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

// TODO: Actually put the implementation here.

