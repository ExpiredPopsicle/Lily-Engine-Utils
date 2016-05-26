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

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
using namespace std;

#include <sys/stat.h>
#include <stdio.h>

#if !_WIN32
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <windows.h>
#include <direct.h>
#endif

#include "console.h"
using namespace ExPop::Console;

#include "filesystem.h"
#include "archive.h"
#include "malstring.h"
#include "thread.h"

namespace ExPop {

    namespace FileSystem {

        // Stick in a fake mutex class and pretend if we aren't using
        // threads at all.
      #if EXPOP_THREADS
        static Threads::Mutex archivesMutex;
      #else
        class FakeMutex { public: void lock() { } void unlock() { } } archivesMutex;
      #endif

        static vector<Archive*> searchArchives;

        bool getAllFiles(const std::string &directory, std::vector<std::string> &names) {

            std::map<std::string, bool> allFiles;

          #if !_WIN32

            // Linux/Unix/whatever way...

            DIR *d = opendir(directory.c_str());

            if(d) {

                struct dirent* de;
                while((de = readdir(d))) {
                    string name(de->d_name);
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
            HANDLE findHandle = FindFirstFile((directory + string("/*")).c_str(), &findData);

            if(findHandle != INVALID_HANDLE_VALUE) {

                string name = findData.cFileName;
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

            string fixedName = fixFileName(directory);
            std::vector<std::string> archivedFiles;

            archivesMutex.lock();

            for(unsigned int i = 0; i < searchArchives.size(); i++) {
                searchArchives[i]->getFileListForDir(fixedName, archivedFiles);
            }

            archivesMutex.unlock();

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

        bool getSubdirectories(const std::string &directory, std::vector<std::string> &names) {
            vector<string> allNames;
            if(!getAllFiles(directory, allNames)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                string fullPath = directory + string("/") + allNames[i];
                if(isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
        }

        bool getNondirectories(const std::string &directory, std::vector<std::string> &names) {
            vector<string> allNames;
            if(!getAllFiles(directory, allNames)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                string fullPath = directory + string("/") + allNames[i];
                if(!isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
        }

        // Windows doesn't define some fstat stuff we need...
#ifndef S_ISDIR
#define S_ISDIR(d) (((d) & S_IFMT) == S_IFDIR)
#endif

        bool fileExists(const std::string &fileName, bool skipArchives) {

            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                return true;
            }

            if(!skipArchives) {

                archivesMutex.lock();

                // Couldn't find it on the filesystem. Try archives.
                string fixedName = fixFileName(fileName);
                for(unsigned int i = 0; i < searchArchives.size(); i++) {
                    if(searchArchives[i]->getFileExists(fixedName) || searchArchives[i]->getDirExists(fixedName)) {
                        archivesMutex.unlock();
                        return true;
                    }
                }

                archivesMutex.unlock();
            }

            return false;
        }

        unsigned int fileChangedTimeStamp(const std::string &fileName) {

            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) == -1) {
              #if EXPOP_CONSOLE
                // out("error") << "Could not stat " << fileName << endl;
              #endif
                return 0;
            }

            return fileStat.st_mtime;
        }

        bool isSymLink(const std::string &fileName) {

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


        bool isDir(const std::string &fileName, bool skipArchives) {
            if(fileName.size() == 0 || fileName == string(".")) {
                return true;
            }

            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                if(S_ISDIR(fileStat.st_mode)) {
                    return true;
                }
            }

            if(!skipArchives) {

                archivesMutex.lock();

                // Couldn't find it on the filesystem. Try archives.
                string fixedName = fixFileName(fileName);
                for(unsigned int i = 0; i < searchArchives.size(); i++) {
                    if(searchArchives[i]->getDirExists(fixedName)) {
                        archivesMutex.unlock();
                        return true;
                    }
                }

                archivesMutex.unlock();
            }

            return false;
        }

        std::string getParentName(const std::string &dirPath) {
            string dp(dirPath);

            int i;
            for(i = int(dp.size()) - 1; i >= 0; i--) {
                if(dp[i] == '/' || dp[i] == '\\') break;
            }

            if(i >= 0) {
                // Cut off the string at the first slash from the end.
                dp[i] = 0;

                return string(dp.c_str());
            }

            return "";
        }

        std::string getBaseName(const std::string &path) {

            string dir = getParentName(path);
            string base = path.c_str() + dir.size();

            // Strip off leading '/'.
            if(base.size() && base[0] == '/') base = base.c_str() + 1;

            return base;
        }

        bool makePath(const std::string &dirPath) {

            if(isDir(dirPath, true)) {

                // Directory already created.
                return true;
            }

            string parent = getParentName(dirPath);
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

        bool makePathForSave(const std::string &fileName) {
            // TODO: Make this cooperate with archives. (Maybe)
            return makePath(getParentName(fileName));
        }

        bool renameFile(const std::string &src, const std::string &dst) {
            // TODO: Make this cooperate with archives. (Maybe)
            return(!rename(src.c_str(), dst.c_str()));
        }

        bool deleteFile(const std::string &fileName) {
            // TODO: Make this cooperate with archives. (Maybe)
            return !remove(fileName.c_str());
        }

        bool recursiveDelete(const std::string &fileName) {

            // remove() handles both directories and files?
            // TODO: Investigate further, otherwise leave if(){} block here.
            // TODO: Make this cooperate with archives. (Maybe)

            if(isDir(fileName)) {
                vector<string> names;
                getAllFiles(fileName, names);

                for(unsigned int i = 0; i < names.size(); i++) {
                    if(!recursiveDelete(fileName + string("/") + names[i])) {
                      #if EXPOP_CONSOLE
                        out("error") << "Recursive delete of \"" << fileName + string("/") + names[i] << "\" failed" << endl;
                      #endif
                        return false;
                    }
                }

            }

            return deleteFile(fileName);
        }

        bool copyFile(const std::string &src, const std::string &dst) {

            // TODO: Make this cooperate with archives. (Maybe only copying OUT of a file)

            ifstream inFile(src.c_str(), ios::in | ios::binary);
            if(!inFile.is_open()) return false;

            ofstream outFile(dst.c_str(), ios::out | ios::binary);
            if(!outFile.is_open()) return false;

            outFile << inFile.rdbuf();

            inFile.close();
            outFile.close();

            return true;
        }

        bool recursiveCopy(const std::string &src, const std::string &dst) {

            // TODO: Make this cooperate with archives. (Maybe)

            if(isDir(src)) {
                if(!makePath(dst)) return false;

                vector<string> names;
                getAllFiles(src, names);

                for(unsigned int i = 0; i < names.size(); i++) {
                    string oldName = src + string("/") + names[i];
                    string newName = dst + string("/") + names[i];
                    if(!recursiveCopy(oldName, newName)) return false;
                }

                return true;

            } else {
                return copyFile(src, dst);
            }
        }

        string stripDataPath(const std::string &path, const std::string &whatToRemove) {
            int removeSize = whatToRemove.size();

            if(path.substr(0, removeSize) == whatToRemove) {
                return path.substr(removeSize);
            }
            return path;
        }

        int getFileSize(const std::string &fileName) {

            // TODO: Should this be replaced with opening the file, seeking
            //   to the end, then recording the position?

            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) == 0) {

                // Found it in the filesystem.
                return fileStat.st_size;

            } else {

                archivesMutex.lock();

                // Couldn't find it on the filesystem. Try archives.
                for(unsigned int i = 0; i < searchArchives.size(); i++) {
                    if(searchArchives[i]->getFileExists(fileName)) {
                        bool ret = searchArchives[i]->getFileSize(fileName);
                        archivesMutex.unlock();
                        return ret;
                    }
                }

                archivesMutex.unlock();

                return -1;
            }
        }

        char *loadFile(const std::string &fileName, int *length, bool addNullTerminator) {

            // Get the file size from file system or archive.
            *length = getFileSize(fileName);

            if(*length <= 0) {

                // It exists in neither!
                return NULL;
            }

            char *data = new char[*length + (addNullTerminator ? 1 : 0)];

            // First, attempt to read everything in from the file system.
            ifstream in(fileName.c_str(), ios::in | ios::binary);
            in.read(data, *length);
            in.close();

            // If that fails, try the archives.
            if(in.fail()) {

                delete[] data;

                archivesMutex.lock();

                // Couldn't load from filesystem. Try archives.
                for(unsigned int i = 0; i < searchArchives.size(); i++) {
                    if(searchArchives[i]->getFileExists(fileName)) {
                        char *ret = searchArchives[i]->loadFile(fileName, length, addNullTerminator);
                        archivesMutex.unlock();
                        return ret;
                    }
                }

                archivesMutex.unlock();

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

        std::string loadFileString(const std::string &fileName) {
            int bufLen = 0;
            char *buf = loadFile(fileName, &bufLen, false);
            if(!buf) {
                return "";
            }
            string ret(buf, bufLen);
            delete[] buf;
            return ret;
        }

        char *loadFilePart(const std::string &fileName, int lengthToRead, int offsetFromStart) {

            int realLength = getFileSize(fileName);
            if(realLength <= 0) {
                return NULL;
            }

            char *buf = new char[lengthToRead];
            memset(buf, 0, lengthToRead);

            // Read from filesystem first.
            ifstream in(fileName.c_str(), ios::in | ios::binary);
            in.seekg(offsetFromStart, ios_base::beg);
            in.read(buf, realLength < lengthToRead ? realLength : lengthToRead);
            in.close();

            if(in.fail()) {

                delete[] buf;

                archivesMutex.lock();

                // Couldn't load from filesystem. Try archives.
                for(unsigned int i = 0; i < searchArchives.size(); i++) {
                    if(searchArchives[i]->getFileExists(fileName)) {
                        char *ret = searchArchives[i]->loadFilePart(fileName, lengthToRead, offsetFromStart);
                        archivesMutex.unlock();
                        return ret;
                    }
                }

                archivesMutex.unlock();

                // Both failed. Ouch. If this happened, something went
                // horribly wrong besides the file just not being there.
                return NULL;
            }

            return buf;

        }

        int saveFile(const std::string &fileName, const char *data, int length, bool mkDirTree) {

            if(mkDirTree) {
                string parentName = getParentName(fileName);
                if(parentName.size()) {
                    makePath(parentName);
                }
            }

            ofstream out;
            out.open(fileName.c_str(), ios::out | ios::binary);

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

        void addArchiveForSearch(const std::string &archiveFileName) {

            archivesMutex.lock();

            // We'll be comparing filenames, so store a fixed version that
            // will be consistent.
            string fixedFileName = fixFileName(archiveFileName);

            // Bail out if it's already in there.
            for(unsigned int i = 0; i < searchArchives.size(); i++) {
                if(searchArchives[i]->getMyFileName() == fixedFileName) {

                    archivesMutex.unlock();
                    return;
                }
            }

            Archive *newArch = new Archive(fixedFileName, false);

            if(newArch->getFailed()) {

                // Hmm... that didn't go well. Not a fatal error, though.
                delete newArch;
              #if EXPOP_CONSOLE
                out("error") << "Failed to open archive file \"" << archiveFileName << "\"" << endl;
              #endif

            } else {

                // Add the new archive.
                searchArchives.push_back(newArch);
            }

            archivesMutex.unlock();

        }

        void removeArchiveForSearch(const std::string &archiveFileName) {

            archivesMutex.lock();

            for(unsigned int i = 0; i < searchArchives.size(); i++) {
                if(searchArchives[i]->getMyFileName() == archiveFileName) {
                    delete searchArchives[i];
                    searchArchives.erase(searchArchives.begin() + i);

                    archivesMutex.unlock();
                    return;
                }
            }

            archivesMutex.unlock();
        }

        void clearSearchArchives(void) {

            archivesMutex.lock();

            for(unsigned int i = 0; i < searchArchives.size(); i++) {
                delete searchArchives[i];
            }

            searchArchives.clear();

            archivesMutex.unlock();
        }

        std::string fixFileName(const std::string &str) {

            // Split the path into directories and the filename.
            vector<string> fileNameParts;
            stringTokenize(str, "\\/", fileNameParts);

            ostringstream outStr;

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

        unsigned int getFileFlags(const std::string &fileName) {

            unsigned int flags = 0;

            archivesMutex.lock();

            for(unsigned int i = 0; i < searchArchives.size(); i++) {
                if(searchArchives[i]->getFileExists(fileName) || searchArchives[i]->getDirExists(fileName)) {
                    flags |= FILEFLAG_ARCHIVED;
                }
            }

            archivesMutex.unlock();

            if(fileExists(fileName, true)) {
                flags |= FILEFLAG_NONARCHIVED;
            }

            return flags;

        }

        void filterFileListByType(const std::string &extension, const std::vector<std::string> &inputList, std::vector<std::string> &outputList) {

            vector<string> extensions;

            stringTokenize(extension, ", ", extensions);

            for(unsigned int i = 0; i < inputList.size(); i++) {
                for(unsigned int j = 0; j < extensions.size(); j++) {
                    if(stringEndsWith(string(".") + extensions[j], inputList[i])) {

                        // This check only does anything if the list is
                        // sorted. Removes doubles.
                        if(!outputList.size() || inputList[i] != outputList[outputList.size() - 1]) {

                            outputList.push_back(inputList[i]);
                        }
                    }
                }
            }
        }

        std::string getCwd(void) {

            // FIXME: Hardcoded directory lengths are bad.

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

            return string(dirBuf);
        }
    }

}

