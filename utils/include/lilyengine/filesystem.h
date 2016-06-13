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
        /// some Windows drive. Max path length is 2048 bytes.
        std::string getCwd(void);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

// archive.h and filesystem.h #include each other, so we want to make
// sure both sets of declarations precede any actual implementation on
// both sides.
#include "archive.h"

namespace ExPop
{
    namespace FileSystem
    {
        // If we have threads disabled, stick in a fake mutex class
        // and pretend if we aren't using threads at all.
      #if EXPOP_ENABLE_THREADS
        typedef Threads::Mutex ArchivesMutexType;
      #else
        class FakeMutex { public: void lock() { } void unlock() { } };
        typedef FakeMutex ArchivesMutexType;
      #endif

        // Global archives mutex.
        inline ArchivesMutexType &getArchivesMutex()
        {
            static ArchivesMutexType archivesMutex;
            return archivesMutex;
        }

        // Justification for ugly global data hack here: The
        // filesystem itself is kind of a global resource, so the
        // archives system "overlay" onto it should be too.
        inline std::vector<Archive*> &getSearchArchives()
        {
            static std::vector<Archive*> searchArchives;
            return searchArchives;
        }

        inline bool getAllFiles(const std::string &directory, std::vector<std::string> &names)
        {
            std::map<std::string, bool> allFiles;

          #if !_WIN32

            // Linux/Unix/whatever way...

            DIR *d = opendir(directory.c_str());

            if(d) {

                struct dirent* de;
                while((de = readdir(d))) {
                    std::string name(de->d_name);
                    // Names that start with a . are hidden in Unix.
                    // This also removes the . and .. directories (which is good).
                    if(name[0] != '.') {
                        //names.push_back(name);
                        allFiles[name] = true;
                    }
                }

                closedir(d);
            }

          #else

            // Windows way...

            WIN32_FIND_DATA findData;
            HANDLE findHandle = FindFirstFile((directory + std::string("/*")).c_str(), &findData);

            if(findHandle != INVALID_HANDLE_VALUE) {

                std::string name = findData.cFileName;
                if(name[0] != '.') {
                    names.push_back(findData.cFileName);
                }
                while(FindNextFile(findHandle, &findData)) {
                    name = findData.cFileName;

                    // See above for reason for not showing . files.
                    if(name[0] != '.') {
                        //names.push_back(name);
                        allFiles[name] = true;
                    }
                }

                FindClose(findHandle);
            }

          #endif

            // Add all matching archive file names.

            std::string fixedName = fixFileName(directory);
            std::vector<std::string> archivedFiles;

            getArchivesMutex().lock();

            for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                getSearchArchives()[i]->getFileListForDir(fixedName, archivedFiles);
            }

            getArchivesMutex().unlock();

            for(unsigned int i = 0; i < archivedFiles.size(); i++) {
                allFiles[archivedFiles[i]] = true;
            }

            // Finally, convert our map we were using to avoid redundancies
            // into a vector list.
            for(std::map<std::string, bool>::iterator i = allFiles.begin(); i != allFiles.end(); i++) {
                names.push_back((*i).first);
            }

            return true;
        }

        inline bool getSubdirectories(const std::string &directory, std::vector<std::string> &names)
        {
            std::vector<std::string> allNames;
            if(!getAllFiles(directory, allNames)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                std::string fullPath = directory + std::string("/") + allNames[i];
                if(isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
        }

        inline bool getNondirectories(const std::string &directory, std::vector<std::string> &names)
        {
            std::vector<std::string> allNames;
            if(!getAllFiles(directory, allNames)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                std::string fullPath = directory + std::string("/") + allNames[i];
                if(!isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
        }

        inline bool fileExists(const std::string &fileName, bool skipArchives)
        {
            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                return true;
            }

            if(!skipArchives) {

                getArchivesMutex().lock();

                // Couldn't find it on the filesystem. Try archives.
                std::string fixedName = fixFileName(fileName);
                for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                    if(getSearchArchives()[i]->getFileExists(fixedName) || getSearchArchives()[i]->getDirExists(fixedName)) {
                        getArchivesMutex().unlock();
                        return true;
                    }
                }

                getArchivesMutex().unlock();
            }

            return false;
        }

        inline unsigned int fileChangedTimeStamp(const std::string &fileName)
        {
            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) == -1) {
                return 0;
            }
            return fileStat.st_mtime;
        }

        inline bool isSymLink(const std::string &fileName)
        {
          #if _WIN32
            // TODO: Now that Windows actually supports symbolic
            // links, we should make this check more accurate. Maybe
            // start at the documentation for CreateSymbolicLink on
            // MSDN.
            return false;
          #else
            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                if(S_ISLNK(fileStat.st_mode)) {
                    return true;
                }
            }
            return false;
          #endif
        }

        // Windows doesn't seem to have the S_ISDIR macro.
        template<typename T>
        inline bool isDirOSFlag(T d)
        {
          #ifdef S_ISDIR
            return S_ISDIR(d);
          #else
            return (((d) & S_IFMT) == S_IFDIR);
          #endif
        }

        inline bool isDir(const std::string &fileName, bool skipArchives)
        {
            if(fileName.size() == 0 || fileName == std::string(".")) {
                return true;
            }

            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                if(isDirOSFlag(fileStat.st_mode)) {
                    return true;
                }
            }

            if(!skipArchives) {

                getArchivesMutex().lock();

                // Couldn't find it on the filesystem. Try archives.
                std::string fixedName = fixFileName(fileName);
                for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                    if(getSearchArchives()[i]->getDirExists(fixedName)) {
                        getArchivesMutex().unlock();
                        return true;
                    }
                }

                getArchivesMutex().unlock();
            }

            return false;
        }

        inline std::string getParentName(const std::string &dirPath)
        {
            std::string dp(dirPath);

            int i;
            for(i = int(dp.size()) - 1; i >= 0; i--) {
                if(dp[i] == '/' || dp[i] == '\\') break;
            }

            if(i >= 0) {
                // Cut off the string at the first slash from the end.
                dp[i] = 0;

                return std::string(dp.c_str());
            }

            return "";
        }

        inline std::string getBaseName(const std::string &path)
        {
            std::string dir = getParentName(path);
            std::string base = path.c_str() + dir.size();

            // Strip off leading '/'.
            if(base.size() && base[0] == '/') base = base.c_str() + 1;

            return base;
        }

        inline bool makePath(const std::string &dirPath)
        {
            if(isDir(dirPath, true)) {

                // Directory already created.
                return true;
            }

            std::string parent = getParentName(dirPath);
            if(makePath(parent)) {

                // Parent exists (or was created. Whatever.)

                int ret;

              #if !_WIN32
                ret = mkdir(dirPath.c_str(), S_IRWXU);
              #else
                ret = _mkdir(dirPath.c_str());
              #endif
                if(ret == -1) {
                    return false;
                }

                return true;

            } else {
                // Something went wrong making the parent.
                return false;
            }
        }

        inline bool renameFile(const std::string &src, const std::string &dst)
        {
            // TODO: Make this cooperate with archives. (Maybe)
            return(!rename(src.c_str(), dst.c_str()));
        }

        inline bool deleteFile(const std::string &fileName)
        {
            // TODO: Make this cooperate with archives. (Maybe)
            return !remove(fileName.c_str());
        }

        inline bool recursiveDelete(const std::string &fileName)
        {
            // remove() handles both directories and files?
            // TODO: Investigate further, otherwise leave if(){} block here.
            // TODO: Make this cooperate with archives. (Maybe)

            if(isDir(fileName)) {
                std::vector<std::string> names;
                getAllFiles(fileName, names);

                for(unsigned int i = 0; i < names.size(); i++) {
                    if(!recursiveDelete(fileName + std::string("/") + names[i])) {
                        return false;
                    }
                }

            }

            return deleteFile(fileName);
        }

        inline bool copyFile(const std::string &src, const std::string &dst)
        {
            // TODO: Make this cooperate with archives. (Maybe only copying OUT of a file)

            std::ifstream inFile(src.c_str(), std::ios::in | std::ios::binary);
            if(!inFile.is_open()) return false;

            std::ofstream outFile(dst.c_str(), std::ios::out | std::ios::binary);
            if(!outFile.is_open()) return false;

            outFile << inFile.rdbuf();

            inFile.close();
            outFile.close();

            return true;
        }

        inline bool recursiveCopy(const std::string &src, const std::string &dst)
        {
            // TODO: Make this cooperate with archives. (Maybe)

            if(isDir(src)) {
                if(!makePath(dst)) return false;

                std::vector<std::string> names;
                getAllFiles(src, names);

                for(unsigned int i = 0; i < names.size(); i++) {
                    std::string oldName = src + std::string("/") + names[i];
                    std::string newName = dst + std::string("/") + names[i];
                    if(!recursiveCopy(oldName, newName)) return false;
                }

                return true;

            } else {
                return copyFile(src, dst);
            }
        }

        inline int getFileSize(const std::string &fileName)
        {
            // TODO: Should this be replaced with opening the file, seeking
            //   to the end, then recording the position?

            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) == 0) {

                // Found it in the filesystem.
                return fileStat.st_size;

            } else {

                getArchivesMutex().lock();

                // Couldn't find it on the filesystem. Try archives.
                for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                    if(getSearchArchives()[i]->getFileExists(fileName)) {
                        bool ret = getSearchArchives()[i]->getFileSize(fileName);
                        getArchivesMutex().unlock();
                        return ret;
                    }
                }

                getArchivesMutex().unlock();

                return -1;
            }
        }

        inline char *loadFile(const std::string &fileName, int *length, bool addNullTerminator)
        {
            // Get the file size from file system or archive.
            *length = getFileSize(fileName);

            if(*length <= 0) {

                // It exists in neither!
                return NULL;
            }

            char *data = new char[*length + (addNullTerminator ? 1 : 0)];

            // First, attempt to read everything in from the file system.
            std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
            in.read(data, *length);
            in.close();

            // If that fails, try the archives.
            if(in.fail()) {

                delete[] data;

                getArchivesMutex().lock();

                // Couldn't load from filesystem. Try archives.
                for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                    if(getSearchArchives()[i]->getFileExists(fileName)) {
                        char *ret = getSearchArchives()[i]->loadFile(fileName, length, addNullTerminator);
                        getArchivesMutex().unlock();
                        return ret;
                    }
                }

                getArchivesMutex().unlock();

                // Both failed. Ouch. If this happened, something went
                // horribly wrong besides the file just not being there.
                return NULL;
            }

            if(addNullTerminator) {
                data[*length] = 0;
                (*length)++;
            }

            return data;

        }

        inline std::string loadFileString(const std::string &fileName)
        {
            int bufLen = 0;
            char *buf = loadFile(fileName, &bufLen, false);
            if(!buf) {
                return "";
            }
            std::string ret(buf, bufLen);
            delete[] buf;
            return ret;
        }

        inline char *loadFilePart(const std::string &fileName, int lengthToRead, int offsetFromStart)
        {
            int realLength = getFileSize(fileName);
            if(realLength <= 0) {
                return NULL;
            }

            char *buf = new char[lengthToRead];
            memset(buf, 0, lengthToRead);

            // Read from filesystem first.
            std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
            in.seekg(offsetFromStart, std::ios_base::beg);
            in.read(buf, realLength < lengthToRead ? realLength : lengthToRead);
            in.close();

            if(in.fail()) {

                delete[] buf;

                getArchivesMutex().lock();

                // Couldn't load from filesystem. Try archives.
                for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                    if(getSearchArchives()[i]->getFileExists(fileName)) {
                        char *ret = getSearchArchives()[i]->loadFilePart(fileName, lengthToRead, offsetFromStart);
                        getArchivesMutex().unlock();
                        return ret;
                    }
                }

                getArchivesMutex().unlock();

                // Both failed. Ouch. If this happened, something went
                // horribly wrong besides the file just not being there.
                return NULL;
            }

            return buf;

        }

        inline int saveFile(const std::string &fileName, const char *data, int length, bool mkDirTree)
        {
            if(mkDirTree) {
                std::string parentName = getParentName(fileName);
                if(parentName.size()) {
                    makePath(parentName);
                }
            }

            std::ofstream out;
            out.open(fileName.c_str(), std::ios::out | std::ios::binary);

            if(out.fail()) {
                return -1;
            }

            out.write(data, length);

            out.close();

            if(out.fail()) {
                return -1;
            }

            return 0;
        }

        inline void addArchiveForSearch(const std::string &archiveFileName)
        {
            getArchivesMutex().lock();

            // We'll be comparing filenames, so store a fixed version that
            // will be consistent.
            std::string fixedFileName = fixFileName(archiveFileName);

            // Bail out if it's already in there.
            for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                if(getSearchArchives()[i]->getMyFileName() == fixedFileName) {
                    getArchivesMutex().unlock();
                    return;
                }
            }

            Archive *newArch = new Archive(fixedFileName, false);

            if(newArch->getFailed()) {

                // Hmm... that didn't go well. Not a fatal error, though.
                delete newArch;

            } else {

                // Add the new archive.
                getSearchArchives().push_back(newArch);
            }

            getArchivesMutex().unlock();

        }

        inline void removeArchiveForSearch(const std::string &archiveFileName)
        {
            getArchivesMutex().lock();

            for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                if(getSearchArchives()[i]->getMyFileName() == archiveFileName) {
                    delete getSearchArchives()[i];
                    getSearchArchives().erase(getSearchArchives().begin() + i);

                    getArchivesMutex().unlock();
                    return;
                }
            }

            getArchivesMutex().unlock();
        }

        inline void clearSearchArchives(void)
        {
            getArchivesMutex().lock();

            for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                delete getSearchArchives()[i];
            }

            getSearchArchives().clear();

            getArchivesMutex().unlock();
        }

        inline std::string fixFileName(const std::string &str)
        {
            // Split the path into directories and the filename.
            std::vector<std::string> fileNameParts;
            stringTokenize(str, "\\/", fileNameParts);

            std::ostringstream outStr;

            // For ".." going above the current directory.
            int numHigherDirectories = 0;

            for(unsigned int i = 0; i < fileNameParts.size(); i++) {

                if(fileNameParts[i] == ".") {

                    // "." for the current directory. Just drop these from
                    // the list. They're redundant.

                    fileNameParts.erase(fileNameParts.begin() + i);
                    i--;

                } else if(fileNameParts[i] == "..") {

                    // ".." for previous directory. Remove this one and the one
                    // before it, UNLESS it's the first one, in which case it's
                    // a directory above the current one.

                    if(i == 0) {

                        // Top level. Remove this directory and count how
                        // far we've gone above the current directory.
                        numHigherDirectories++;
                        fileNameParts.erase(fileNameParts.begin());

                        // I know this will cause the unsigned int to wrap
                        // around, but it'll wrap back when it increments
                        // again. Please don't kill me.
                        i--;

                    } else {

                        // Remove the previous directory.
                        fileNameParts.erase(fileNameParts.begin() + (i-1));

                        // Remove THIS directory.
                        fileNameParts.erase(fileNameParts.begin() + (i-1));

                        // Unsigned weirdness: We're at at least 1 here, so
                        // the most this can do is wrap around for -1. The
                        // next iteration of the for loop will bring it back
                        // to zero.
                        i -= 2;
                    }
                }
            }

            for(int i = 0; i < numHigherDirectories; i++) {
                // Add "../" for as many directories we ended up going
                // above the current one.
                outStr << "../";
            }

            for(unsigned int i = 0; i < fileNameParts.size(); i++) {
                // Add the current name.
                outStr << fileNameParts[i];

                // If this isn't the last one, then it's not at the file
                // name yet.
                if(i != fileNameParts.size() - 1) {
                    outStr << "/";
                }
            }

            return outStr.str();
        }

        inline unsigned int getFileFlags(const std::string &fileName)
        {
            unsigned int flags = 0;

            getArchivesMutex().lock();

            for(unsigned int i = 0; i < getSearchArchives().size(); i++) {
                if(getSearchArchives()[i]->getFileExists(fileName) || getSearchArchives()[i]->getDirExists(fileName)) {
                    flags |= FILEFLAG_ARCHIVED;
                }
            }

            getArchivesMutex().unlock();

            if(fileExists(fileName, true)) {
                flags |= FILEFLAG_NONARCHIVED;
            }

            return flags;

        }

        inline void filterFileListByType(const std::string &extension, const std::vector<std::string> &inputList, std::vector<std::string> &outputList)
        {
            std::vector<std::string> extensions;

            stringTokenize(extension, ", ", extensions);

            for(unsigned int i = 0; i < inputList.size(); i++) {
                for(unsigned int j = 0; j < extensions.size(); j++) {
                    if(stringEndsWith(std::string(".") + extensions[j], inputList[i])) {

                        // This check only does anything if the list is
                        // sorted. Removes doubles.
                        if(!outputList.size() || inputList[i] != outputList[outputList.size() - 1]) {

                            outputList.push_back(inputList[i]);
                        }
                    }
                }
            }
        }

        inline std::string getCwd(void)
        {
            // FIXME: Hardcoded directory lengths are bad, but all the
            // API we use for each platform takes a buffer size, so
            // we're not at risk of running over. Just giving a bad
            // answer.
            char dirBuf[2048];

          #if _WIN32

            GetCurrentDirectory(2048, dirBuf);

            // Convert backslashes to forward slashes for consistency.
            size_t len = strlen(dirBuf);
            for(size_t i = 0; i < len; i++) {
                if(dirBuf[i] == '\\') {
                    dirBuf[i] = '/';
                }
            }

          #else

            if(!getcwd(dirBuf, 2048)) {
                return "";
            }

          #endif

            return std::string(dirBuf);
        }

    }
}


