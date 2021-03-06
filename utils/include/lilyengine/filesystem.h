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

// Filesystem access module.

// TODO: Check this:
// http://www.reddit.com/r/gamedev/comments/s7y83/saving_configuration_files_savegame_data_etc_on/

// TODO: Maybe a thing to get the data directory, as it's been
// configured.

// TODO: Symlink detection on Windows?

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "malstring.h"
#include "thread.h"

// Compensate for broken Cygwin headers.
#if __CYGWIN__
#define NAME_MAX FILENAME_MAX
#endif

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <string>
#include <cstdlib>

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

#include "config.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    class ZipFile;

    namespace FileSystem
    {
        /// Get all the files and subdirectories in a directory.
        /// Returns true on success and false on failure.
        bool getAllFiles(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden = false);

        /// Get all the subdirectories in a directory. Returns true if
        /// successful, false otherwise.
        bool getSubdirectories(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden = false);

        /// Get all the non-directory files in a directory. Returns
        /// true if successful, false otherwise.
        bool getNondirectories(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden = false);

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
            FILEFLAG_OVERLAY     = 4,
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
        int64_t getFileSize(const std::string &fileName);

        /// Load a file to a new buffer (caller takes ownership of the
        /// buffer). Buffer length is stored in length. Use
        /// addNullTerminator if you intend to use the result as a string.
        /// length will be changed to reflect the size of the returned data.
        char *loadFile(const std::string &fileName, int64_t *length);

        /// Load a file and just return it as an std::string.
        std::string loadFileString(const std::string &fileName);

        /// Load a part of a file into a new buffer. Only do this if
        /// you already know the length you're dealing with. If it
        /// hits a file in an archive it may start reading into the
        /// next file's header and the next file.
        char *loadFilePart(const std::string &fileName, int64_t lengthToRead, int64_t offsetFromStart = 0);

        /// Saves a buffer to a file. Returns -1 on failure or 0 on
        /// success.
        int saveFile(const std::string &fileName, const char *data, int64_t length, bool mkDirTree = false);

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

        /// Returns true if the path given is a full path.
        bool isFullPath(const std::string &path);

        /// Returns the given path, converted into a full path.
        std::string makeFullPath(const std::string &path);

        /// Returns the given path, converted into a relative path
        /// from the current working directory.
        std::string makeRelativePath(const std::string &path);

        /// Open a file for reading. May search inside archives.
        std::shared_ptr<std::istream> openReadFile(const std::string &fileName);

        /// Add the contents of a zip file as an overlay to the
        /// existing filesystem (real files override zip contents).
        void mountZipFile(std::shared_ptr<ExPop::ZipFile> zf, const std::string &location);

        /// Overlay a zip file by its filename, in the directory it
        /// exists in.
        std::shared_ptr<ExPop::ZipFile> mountZipFile(const std::string &filename);

        /// Mount a directory as an overlay to another directory.
        /// Files in the target directory override files in the source
        /// directory.
        void mountOverlay(
            const std::string &sourcePath,
            const std::string &mountPoint);

        /// Unmount all overlays.
        ///
        /// Note: We don't support granular un-mounting because
        /// information is lost when multiple archives are mounted as
        /// overlays to each other.
        ///
        /// FIXME: At least keep a list of mounted overlay files and
        /// where they're mounted.
        void unmountAll(void);

        // TODO: Function to mount a directory as an overlay onto
        // another directory.
        //
        // Note: If we just add all the files in the tree that exists
        // at the time we call the mount command, we will not be able
        // to respond to changes in the overlay filesystem. This may
        // require making a whole new system for directory overlays.

        /// Get a path to the executable itself. FIXME: Do NOT use if
        /// the current directory has changed sine the program
        /// started.
        inline std::string getExecutablePath(const char *argv0);
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

// archive.h and filesystem.h #include each other, so we want to make
// sure both sets of declarations precede any actual implementation on
// both sides.
#include "archive.h"
#include "deflate/zipfile.h"

namespace ExPop
{
    class ZipFile;

    namespace FileSystem
    {
        // ----------------------------------------------------------------------
        // Internal types.

        class ArchiveTreeNode
        {
        public:
            std::map<std::string, std::shared_ptr<ArchiveTreeNode> > children;

            ArchiveTreeNode *resolvePath(
                const std::vector<std::string> &pathParts,
                size_t pathPartsIndex,
                bool buildPath = false);

            ArchiveTreeNode *resolvePath(
                const std::string &path,
                bool buildPath = false);

            void dump(size_t indent = 0);



            std::shared_ptr<ZipFile> zipFile;
            std::string filenameInZipFile;
        };

        inline ArchiveTreeNode *ArchiveTreeNode::resolvePath(
            const std::vector<std::string> &pathParts,
            size_t pathPartsIndex,
            bool buildPath)
        {
            if(pathPartsIndex == pathParts.size()) {
                return this;
            }

            auto it = children.find(pathParts[pathPartsIndex]);

            if(it != children.end()) {

                return it->second->resolvePath(pathParts, pathPartsIndex + 1, buildPath);

            } else if(buildPath) {

                children[pathParts[pathPartsIndex]] =
                    std::shared_ptr<ArchiveTreeNode>(new ArchiveTreeNode);
                return children[pathParts[pathPartsIndex]]->resolvePath(
                    pathParts, pathPartsIndex + 1, buildPath);

            }

            return nullptr;
        }

        inline ArchiveTreeNode *ArchiveTreeNode::resolvePath(
            const std::string &path,
            bool buildPath)
        {
            std::string fullPath = makeFullPath(path);
            std::vector<std::string> fullPathParts;
            stringTokenize(fullPath, "/", fullPathParts);
            return resolvePath(fullPathParts, 0, buildPath);
        }

        inline void ArchiveTreeNode::dump(size_t indent)
        {
            for(auto it = children.begin(); it != children.end(); it++) {
                for(size_t i = 0; i < indent; i++) {
                    std::cout << "  ";
                }
                std::cout << it->first << std::endl;
                it->second->dump(indent + 1);
            }
        }

        inline std::shared_ptr<ArchiveTreeNode> getRootArchiveTreeNode()
        {
            static std::shared_ptr<ArchiveTreeNode> root(new ArchiveTreeNode);
            return root;
        }

        struct OverlayOverride
        {
            std::string mountPoint;
            std::string realPath;
        };

        inline std::vector<OverlayOverride> &getOverlayList(void)
        {
            static std::vector<OverlayOverride> overlayList;
            return overlayList;
        }

        inline void mountOverlay(
            const std::string &sourcePath,
            const std::string &mountPoint)
        {
            OverlayOverride overlay;
            overlay.mountPoint = makeFullPath(mountPoint);
            overlay.realPath = makeFullPath(sourcePath);
            getOverlayList().push_back(overlay);
        }

        inline void findOverlayPath(
            const std::string &path,
            std::vector<std::string> &foundPaths)
        {
            std::vector<OverlayOverride> &overlays = getOverlayList();
            std::string fullPath = makeFullPath(path);

            // Check against all overlays to see if we need to rewrite
            // this.
            for(size_t i = 0; i < overlays.size(); i++) {
                if(stringStartsWith(
                        overlays[i].mountPoint,
                        fullPath))
                {
                    std::string rewrittenPath =
                        overlays[i].realPath +
                        fullPath.substr(overlays[i].mountPoint.size());

                    // Found a file in this overlay.
                    if(fileExists(rewrittenPath, true)) {
                        foundPaths.push_back(rewrittenPath);
                    }

                    findOverlayPath(rewrittenPath, foundPaths);
                }
            }
        }

        // ----------------------------------------------------------------------
        // Functions that just juggle data and don't touch the file
        // system directly (but may call our own functions that do).
        // These ones shouldn't need to be altered for
        // platform-specific things ever.

        inline bool getSubdirectories(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden)
        {
            std::vector<std::string> allNames;
            if(!getAllFiles(directory, allNames, showUnixHidden)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                std::string fullPath = directory + std::string("/") + allNames[i];
                if(isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
        }

        inline bool getNondirectories(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden)
        {
            std::vector<std::string> allNames;
            if(!getAllFiles(directory, allNames, showUnixHidden)) return false;

            for(unsigned int i = 0; i < allNames.size(); i++) {
                std::string fullPath = directory + std::string("/") + allNames[i];
                if(!isDir(fullPath)) {
                    names.push_back(allNames[i]);
                }
            }
            return true;
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
            std::string base = path.substr(dir.size());

            // Strip off leading '/'.
            if(base.size() && base[0] == '/') base = base.c_str() + 1;

            return base;
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

            std::string ret = outStr.str();

            // Little hack to restore the root slash that we may have
            // dropped in the tokenization stage.
            if(str.size() && str[0] == '/') {
                ret = "/" + ret;
            }

            return ret;
        }

        inline void filterFileListByType(
            const std::string &extension,
            const std::vector<std::string> &inputList,
            std::vector<std::string> &outputList)
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

        inline bool isFullPath(const std::string &path)
        {
            std::string testPath = fixFileName(path);

            if(testPath.size()) {

                if(testPath[0] == '/') {
                    // Unix path starting at root.
                    return true;
                }

                if((testPath[0] >= 'A' && testPath[0] <= 'Z') ||
                    (testPath[0] >= 'a' && testPath[1] <= 'z'))
                {
                    if(testPath.size() > 1) {

                        if(testPath[1] == ':') {
                            // Windows drive letter.
                            return true;
                        }

                    }

                }
            }

            return false;
        }

        inline std::string makeFullPath(const std::string &path)
        {
            if(isFullPath(path)) {
                return fixFileName(path);
            }

            return fixFileName(getCwd() + "/" + path);
        }

        inline std::string makeRelativePath(const std::string &path)
        {
            // Get full paths.
            std::string fullPath = makeFullPath(path);
            std::string cwd = getCwd();

            // Tokenize.
            std::vector<std::string> cwdParts;
            stringTokenize(cwd, "/", cwdParts);

            std::vector<std::string> pathParts;
            stringTokenize(fullPath, "/", pathParts);

            // Strip off common elements from the front.
            while(cwdParts.size() && pathParts.size() && cwdParts[0] == pathParts[0]) {
                cwdParts.erase(cwdParts.begin());
                pathParts.erase(pathParts.begin());
            }

            // Now just turn every directory left in the cwd into a
            // "..".
            for(size_t i = 0; i < cwdParts.size(); i++) {
                pathParts.insert(pathParts.begin(), "..");
            }

            // Re-assemble the full path.
            std::ostringstream retStr;
            for(size_t i = 0; i < pathParts.size(); i++) {
                retStr << pathParts[i];
                if(i + 1 < pathParts.size()) {
                    retStr << "/";
                }
            }

            return retStr.str();
        }

        // ----------------------------------------------------------------------
        // Things that actually touch the system. OS-specific stuff
        // happens here.

        inline bool getAllFiles(
            const std::string &directory,
            std::vector<std::string> &names,
            bool showUnixHidden)
        {
            std::map<std::string, bool> allFiles;

          #if !_WIN32

            // Linux/Unix/whatever way...

            DIR *d = opendir(directory.c_str());

            if(d) {

                struct dirent* de;
                while((de = readdir(d))) {

                    std::string name(de->d_name);

                    if(name.size()) {

                        if(name == "." || name == "..") {
                            continue;
                        }

                        // Names that start with a . are hidden in Unix.
                        if(name[0] != '.' || showUnixHidden) {
                            allFiles[name] = true;
                        }
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

                    if(name.size()) {

                        if(name == "." || name == "..") {
                            continue;
                        }

                        // Names that start with a . are hidden in Unix.
                        if(name[0] != '.' || showUnixHidden) {
                            allFiles[name] = true;
                        }
                    }
                }

                FindClose(findHandle);
            }

          #endif

            // Zip archives.
            ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(directory);
            if(node) {
                for(auto it = node->children.begin(); it != node->children.end(); it++) {
                    allFiles[it->first] = true;
                }
            }

            // Overlays.
            std::vector<std::string> overlayPaths;
            findOverlayPath(directory, overlayPaths);
            for(size_t i = 0; i < overlayPaths.size(); i++) {
                if(overlayPaths[i] != makeFullPath(directory)) {
                    std::vector<std::string> namesInOverlay;
                    getAllFiles(overlayPaths[i], namesInOverlay, showUnixHidden);
                    for(size_t n = 0; n < namesInOverlay.size(); n++) {
                        allFiles[namesInOverlay[n]] = true;
                    }
                }
            }

            // Finally, convert our map we were using to avoid redundancies
            // into a vector list.
            for(std::map<std::string, bool>::iterator i = allFiles.begin(); i != allFiles.end(); i++) {
                names.push_back((*i).first);
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

                // Zip archives.
                ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(fileName);
                if(node) {
                    return true;
                }

                // Overlays.
                std::vector<std::string> overlayPaths;
                findOverlayPath(fileName, overlayPaths);
                for(size_t i = 0; i < overlayPaths.size(); i++) {
                    if(fileExists(overlayPaths[i], skipArchives)) {
                        return true;
                    }
                }
            }

            return false;
        }

        inline unsigned int fileChangedTimeStamp(const std::string &fileName)
        {
            // Real file.
            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) != -1) {
                return fileStat.st_mtime;
            }

            // FIXME: Add zips.

            // Overlays.
            std::vector<std::string> overlayPaths;
            findOverlayPath(fileName, overlayPaths);
            for(size_t i = 0; i < overlayPaths.size(); i++) {
                unsigned int timeStamp = fileChangedTimeStamp(overlayPaths[i]);
                if(timeStamp) {
                    return timeStamp;
                }
            }

            return 0;
        }

        inline bool isSymLink(const std::string &fileName)
        {
          #if _WIN32
            // TODO: Now that Windows actually supports symbolic
            // links, we should make this check more accurate. Maybe
            // start at the documentation for CreateSymbolicLink on
            // MSDN.
          #else
            struct stat fileStat;
            if(!stat(fileName.c_str(), &fileStat)) {
                if(S_ISLNK(fileStat.st_mode)) {
                    return true;
                }
            }
          #endif

            // Overlays.
            std::vector<std::string> overlayPaths;
            findOverlayPath(fileName, overlayPaths);
            for(size_t i = 0; i < overlayPaths.size(); i++) {
                if(isSymLink(overlayPaths[i])) {
                    return true;
                }
            }

            return false;
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

                // Overlays.
                std::vector<std::string> overlayPaths;
                findOverlayPath(fileName, overlayPaths);
                for(size_t i = 0; i < overlayPaths.size(); i++) {
                    if(isDir(overlayPaths[i], skipArchives)) {
                        return true;
                    }
                }

                // Zip archives.
                ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(fileName);
                if(node) {
                    // This assumes no empty directories inside
                    // archives. So having no children = file, and
                    // having children = directory. We must be aware
                    // of this when mounting the zip file.
                    return !!node->children.size();
                }
            }

            return false;
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
            return(!rename(src.c_str(), dst.c_str()));
        }

        inline bool deleteFile(const std::string &fileName)
        {
            return !remove(fileName.c_str());
        }

        inline bool recursiveDelete(const std::string &fileName)
        {
            // remove() handles both directories and files?
            // TODO: Investigate further, otherwise leave if(){} block here.

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

            std::shared_ptr<std::istream> inFile = openReadFile(src);
            if(!inFile) return false;

            std::ofstream outFile(dst.c_str(), std::ios::out | std::ios::binary);
            if(!outFile.is_open()) return false;

            outFile << inFile->rdbuf();

            outFile.close();

            return true;
        }

        inline bool recursiveCopy(const std::string &src, const std::string &dst)
        {
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

        inline int64_t getFileSize(const std::string &fileName)
        {
            // TODO: Should this be replaced with opening the file, seeking
            //   to the end, then recording the position?

            struct stat fileStat;
            if(stat(fileName.c_str(), &fileStat) == 0) {

                // Found it in the filesystem.
                return fileStat.st_size;

            } else {

                // Overlays.
                std::vector<std::string> overlayPaths;
                findOverlayPath(fileName, overlayPaths);
                for(size_t i = 0; i < overlayPaths.size(); i++) {
                    int64_t fileSize = getFileSize(overlayPaths[i]);
                    if(fileSize != -1) {
                        return fileSize;
                    }
                }

                // Zip archives.
                ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(fileName);
                if(node) {
                    std::shared_ptr<ZipFile> zf = node->zipFile;
                    if(zf) {
                        return zf->getFileSize(node->filenameInZipFile);
                    }
                }

            }

            return -1;
        }

        inline std::shared_ptr<std::istream> openReadFile(const std::string &fileName)
        {
            // Attempt to open the actual file first.
            std::shared_ptr<std::istream> realFile(
                new std::fstream(
                    fileName,
                    std::ios_base::in | std::ios_base::binary));

            if(!realFile->fail()) {
                return realFile;
            }

            // Overlays.
            std::vector<std::string> overlayPaths;
            findOverlayPath(fileName, overlayPaths);
            for(size_t i = 0; i < overlayPaths.size(); i++) {
                std::shared_ptr<std::istream> foundFile = openReadFile(overlayPaths[i]);
                if(foundFile) {
                    return foundFile;
                }
            }

            // Fallback to zip archives.
            ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(fileName);
            if(node) {
                std::shared_ptr<ZipFile> zf = node->zipFile;
                if(zf) {
                    return zf->openFile(node->filenameInZipFile);
                }
            }

            // All attempts failed.
            return nullptr;
        }

        struct LoadFileTempBuf
        {
            struct LoadFileTempBuf *next;
            char data[1024];
        };

        inline char *loadFile(const std::string &fileName, int64_t *length)
        {
            // Needlessly complicated file loading system
            // explanation...
            //
            // We can't just allocate one big buffer based on the
            // return value from getFileSize, because maliciously
            // constructed archive files (which we support reading
            // directly from) can have directory entries indicating
            // 2^64 or whatever sized files that'll make us hit an
            // allocation error pretty darn quick.
            //
            // I also decided not to just read in data while doubling
            // the size of a single buffer because, for larger files,
            // that turns into a *lot* of copying.
            //
            // So this approach just does a ton of smaller allocations
            // and then copies them one more time into a single buffer
            // at the end.
            //
            // Thanks, AFL, for discovering that little problem.

            // Fail to load a file if it doesn't exist.
            if(!fileExists(fileName)) {
                return NULL;
            }

            int64_t realLength = 0;

            std::shared_ptr<std::istream> in = openReadFile(fileName);

            LoadFileTempBuf *startBuf = new LoadFileTempBuf;
            LoadFileTempBuf *currentBuf = startBuf;

            if(in) {

                while(in->good()) {

                    // Load data into the current buffer.
                    in->read(currentBuf->data, sizeof(currentBuf->data));
                    realLength += in->gcount();

                    // Make another allocation for the next buffer.
                    LoadFileTempBuf *nextBuf = new LoadFileTempBuf;
                    currentBuf->next = nextBuf;
                    currentBuf = nextBuf;
                    nextBuf->next = nullptr;
                }

                // Finally, move everything into one contiguous
                // buffer.
                char *finalData = new char[realLength];
                size_t lengthLeft = (size_t)realLength;
                size_t writePosition = 0;
                currentBuf = startBuf;

                while(lengthLeft) {

                    int64_t bytesToCopy =
                        lengthLeft > sizeof(LoadFileTempBuf::data) ?
                        sizeof(LoadFileTempBuf::data) : lengthLeft;

                    memcpy(&finalData[writePosition], currentBuf->data, bytesToCopy);

                    writePosition += bytesToCopy;
                    lengthLeft -= bytesToCopy;

                    currentBuf = currentBuf->next;
                }

                // Cleanup.
                currentBuf = startBuf;
                while(currentBuf) {
                    LoadFileTempBuf *oldCurrentBuf = currentBuf;
                    currentBuf = currentBuf->next;
                    delete oldCurrentBuf;
                }

                *length = realLength;
                return finalData;
            }

            // Read failed.
            return nullptr;
        }

        inline std::string loadFileString(const std::string &fileName)
        {
            int64_t bufLen = 0;
            char *buf = loadFile(fileName, &bufLen);
            if(!buf) {
                return "";
            }

            // Note: This will get '\0' characters that would normally
            // be treated as the end of a C-style string, in case you
            // want to load some binary blob into an std::string.
            std::string ret(buf, bufLen);

            delete[] buf;
            return ret;
        }

        inline char *loadFilePart(
            const std::string &fileName, int64_t lengthToRead, int64_t offsetFromStart)
        {
            int64_t realLength = getFileSize(fileName);
            if(realLength <= 0) {
                return NULL;
            }

            char *buf = new char[lengthToRead];
            memset(buf, 0, lengthToRead);

            // Read from filesystem first.
            std::shared_ptr<std::istream> in = openReadFile(fileName);
            if(in) {
                in->seekg(offsetFromStart);
                in->read(buf, realLength < lengthToRead ? realLength : lengthToRead);
                return buf;
            }

            // Read failed.
            delete[] buf;
            return nullptr;
        }

        inline int saveFile(
            const std::string &fileName, const char *data,
            int64_t length, bool mkDirTree)
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

        inline unsigned int getFileFlags(const std::string &fileName)
        {
            unsigned int flags = 0;

            // Zip archives.
            ArchiveTreeNode *node = getRootArchiveTreeNode()->resolvePath(fileName);
            if(node) {
                flags |= FILEFLAG_ARCHIVED;
            }

            // Overlays.
            std::vector<std::string> overlayPaths;
            findOverlayPath(fileName, overlayPaths);
            for(size_t i = 0; i < overlayPaths.size(); i++) {
                if(fileExists(overlayPaths[i])) {
                    flags |= FILEFLAG_OVERLAY;
                }
            }

            // Non-archives.
            if(fileExists(fileName, true)) {
                flags |= FILEFLAG_NONARCHIVED;
            }

            return flags;
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

        inline void mountZipFile(std::shared_ptr<ExPop::ZipFile> zf, const std::string &location)
        {
            std::vector<std::string> fileList = zf->getFileList();
            std::string locationPrefix = location.size() ? (location + "/") : "";

            for(size_t i = 0; i < fileList.size(); i++) {

                std::string overlayPathName =
                    ExPop::FileSystem::fixFileName(locationPrefix + fileList[i]);

                ExPop::FileSystem::ArchiveTreeNode *node =
                    ExPop::FileSystem::getRootArchiveTreeNode()->resolvePath(overlayPathName, true);

                node->filenameInZipFile = fileList[i];
                node->zipFile = zf;
            }
        }

        inline std::shared_ptr<ExPop::ZipFile> mountZipFile(const std::string &filename)
        {
            std::string directoryName = ExPop::FileSystem::getParentName(filename);
            std::shared_ptr<ExPop::ZipFile> zf(new ExPop::ZipFile(filename));
            mountZipFile(zf, directoryName);
            return zf;
        }

        inline void unmountAll(void)
        {
            getRootArchiveTreeNode()->children.clear();
            getOverlayList().clear();
        }

        inline std::string getExecutablePath(const char *argv0)
        {
          #if defined _WIN32
            char pathSeparator[2] = ";";
          #else
            char pathSeparator[2] = ":";
          #endif

            // Full path from the root directory.
            if(isFullPath(argv0)) {
                return fixFileName(argv0);
            }

            // Path in the form of "./foo" or "foo/bar". Note that
            // this will break if the directory has changed since
            // starting the program!
            if(getBaseName(argv0) != argv0) {
                return makeFullPath(argv0);
            }

            // Now search system path...
            const char *systemPath = getenv("PATH");
            if(systemPath) {

                std::vector<std::string> splitPath;
                stringTokenize(systemPath, pathSeparator, splitPath, false);

              #ifdef _WIN32
                // Windows allows running stuff just from the current
                // directory.
                splitPath.push_back(".");
              #endif

                for(size_t i = 0; i < splitPath.size(); i++) {
                    std::string maybePath = fixFileName(splitPath[i] + "/" + argv0);
                    if(fileExists(maybePath)) {
                        // TODO: Check to see if it's actually executable!
                        return maybePath;
                    }
                }
            }

            return "";
        }

    }
}


