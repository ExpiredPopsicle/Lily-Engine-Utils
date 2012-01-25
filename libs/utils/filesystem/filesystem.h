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

#pragma once

#include <string>
#include <vector>

namespace ExPop {

    namespace FileSystem {

        /** @brief Get all the files and subdirectories in a directory.
         * @param directory The directory to scan.
         * @param names A place to stick the file names.
         * @return true if successful, false otherwise.
         * */
        bool getAllFiles(const std::string &directory, std::vector<std::string> &names);

        /** @brief Get all the subdirectories in a directory.
         * @param directory The directory to scan.
         * @param names A place to stick the file names.
         * @return true if successful, false otherwise.
         * */
        bool getSubdirectories(const std::string &directory, std::vector<std::string> &names);

        /** @brief Get all the non-directory files in a directory.
         * @param directory The directory to scan.
         * @param names A place to stick the file names.
         * @return true if successful, false otherwise.
         * */
        bool getNondirectories(const std::string &directory, std::vector<std::string> &names);

        /** @brief Determine if a file exists.
         * @param fileName The name of the file to check.
         * @param skipArchives True to not check archives.
         * @return true if it exists.
         * */
        bool fileExists(const std::string &fileName, bool skipArchives = false);

        /// Get a file timestamp as the number of seconds since the
        /// Unix epoch. Doesn't work with stuff stored in archives
        /// because timestamps aren't stored in archives.
        unsigned int fileChangedTimeStamp(const std::string &fileName);

        /** @brief Determine if a file is a directory.
         * @param fileName The name of the file to check.
         * @return true if it is a directory.
         * */
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

        /** @brief Get the name of the parent directory to the path.
         * @param dirPath The path to the file or directory, including it.
         * @return The name of the parent directory, or "" if it's at the highest level.
         * */
        std::string getParentName(const std::string &dirPath);

        /// Get the name of the file without the directory.
        std::string getBaseName(const std::string &path);

        /** @brief Recursively make directories to create a given path.
         * @param dirPath The path to construct.
         * @return true if successful, false otherwise.
         * */
        bool makePath(const std::string &dirPath);

        /** @brief Recursively make directories so that the given file can be saved.
         * @param fileName The file name, with full path.
         * @return true if successful, false otherwise.
         * */
        bool makePathForSave(const std::string &fileName);

        /** @brief Renames (moves) a file.
         * @param src The old (source) file name.
         * @param dst The new (destination) file name.
         * @return true if successful, false otherwise.
         * */
        bool renameFile(const std::string &src, const std::string &dst);

        /** @brief Deletes a file (including empty directories).
         * @param fileName The file to delete.
         * @return true if successful, false otherwise.
         * */
        bool deleteFile(const std::string &fileName);

        /** @brief Deletes a file or directory and all files or directories inside.
         * @param fileName The file to delete.
         * @return true if successful, false otherwise.
         * */
        bool recursiveDelete(const std::string &fileName);

        /** @brief Copies a file.
         * @param src The old (source) file name.
         * @param dst The new (destination) file name.
         * @return true if successful, false otherwise.
         * */
        bool copyFile(const std::string &src, const std::string &dst);

        /** @brief Copies an entire tree of files and directories.
         * @param src The old (source) file name.
         * @param dst The new (destination) file name.
         * @return true if successful, false otherwise.
         * */
        bool recursiveCopy(const std::string &src, const std::string &dst);

        /** @brief Takes out the first part of a path.
         * Used for taking out the "games/<game name>/" part of a path, if
         * it's there.
         * @param path The path to start with.
         * @param whatToRemove The string to remove from the start.
         * @return The string with the part removed, or just the string if
         *   that part wasn't there to be removed.
         * */
        std::string stripDataPath(const std::string &path, const std::string &whatToRemove);

        /// Get the size of a file in bytes.
        int getFileSize(const std::string &fileName);

        /// Load a file to a new buffer (caller takes ownership of the
        /// buffer). Buffer length is stored in length. Use
        /// addNullTerminator if you intend to use the result as a string.
        /// length will be changed to reflect the size of the returned data.
        char *loadFile(const std::string &fileName, int *length, bool addNullTerminator = false);

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
        void filterFileListByType(const std::string &extension, const std::vector<std::string> &inputList, std::vector<std::string> &outputList);
    }
}
