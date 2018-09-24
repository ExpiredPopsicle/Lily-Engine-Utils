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

// Threaded asset loader for streaming assets.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "config.h"

#if EXPOP_ENABLE_THREADS

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#if !_WIN32
#include <unistd.h>
#endif

#include "thread.h"
#include "filesystem.h"

#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_THREADS
namespace ExPop
{
    /// Background asset loading system.
    class AssetLoader
    {
    public:

        AssetLoader(void);
        ~AssetLoader(void);

        // These are inferred from the state of LoadRequest.
        enum LoadStatus {

            LOADSTATUS_WAITING, // LoadRequest hasn't even started
                                // yet.

            LOADSTATUS_LOADING, // LoadRequest has started but not
                                // finished.

            LOADSTATUS_FAIL,    // LoadRequest has finished, but data
                                // is NULL.

            LOADSTATUS_SUCCESS, // LoadRequest has finished and data
                                // is not NULL.

        };

        /// Request some data from the loading system. *status will be
        /// set according to what stage the load is on. It will return
        /// the data if it is available, or NULL if there was an error
        /// or the load is still pending. If a load was not pending,
        /// it will be added to the list of stuff to load with the
        /// given priority. If the priority is higher than the
        /// existing load priority, then the priority will be
        /// changed. Buffers returned by this function may be deleted
        /// when calling ageData(). So get your buffer and either copy
        /// it and save it or be done with it by the time ageData() is
        /// called.
        char *requestData(
            const std::string &fileName,
            float priority = 100,
            int *lengthOut = NULL,
            LoadStatus *status = NULL,
            int start = -1,
            int length = -1);

        /// Increment age counters on finished LoadRequests and delete
        /// them if they've gone too long without someone accessing
        /// them. This may make some buffers returned by requestData()
        /// invalid.
        void ageData(int ageAmount = 1);

        /// True if we're loading or waiting for something right now.
        bool loading(void);

    private:

        /// Internal representation of a pending or finished load
        /// request.
        class LoadRequest {
        public:

            LoadRequest(
                const std::string &fileName,
                float priority = 100.0,
                int start = -1,
                int length = -1);

            ~LoadRequest(void);

            int start;
            int length;

            std::string fileName;
            float priority;
            bool started;
            bool done;
            char *loadedBuffer;
            int64_t loadedBufferLength;
            int age;

        };

        /// We use these for hashing load requests.
        class LoadRequestDef {
        public:
            std::string fileName;
            int start;
            int length;

            inline LoadRequestDef(void)
            {
                start = -1;
                length = -1;
            }

            inline LoadRequestDef(
                std::string fileName,
                int start,
                int length)
            {
                this->fileName = fileName;
                this->start = start;
                this->length = length;
            }

            inline bool operator==(const LoadRequestDef &otherDef) const
            {
                return
                    start == otherDef.start &&
                    length == otherDef.length &&
                    fileName == otherDef.fileName;
            }

            inline bool operator<(const LoadRequestDef &otherDef) const
            {
                if(fileName < otherDef.fileName) return true;
                if(fileName > otherDef.fileName) return false;

                // Filenames must be the same.

                if(start < otherDef.start) return true;
                if(start > otherDef.start) return false;

                // Starting points must be the same.

                if(length < otherDef.length) return true;

                return false;
            }
        };

        /// Only the loading thread itself should call this. Process a
        /// single load request. Returns true if it did something.
        /// False otherwise.
        bool processLoadRequest(void);

        /// Lock this mutex before touching any list or hash table of
        /// LoadRequests for reading or writing.
        Threads::Mutex loadListMutex;

        /// List of loads waiting to start. The order of this will get
        /// all jumbled up as the loading thread picks stuff out of it
        /// and moves things to fill the slot.
        std::vector<LoadRequest*> pendingLoads;

        /// List of all finished loads. These will begin to age if
        /// they are not accessed and eventually be freed.
        std::vector<LoadRequest*> finishedLoads;

        /// What the loader thread is working on right now. It's
        /// allowed to mess with this as much as it wants, so don't
        /// change it outside of the loader thread.
        LoadRequest *currentLoad;

        struct LoadRequestHash
        {
            size_t operator()(const AssetLoader::LoadRequestDef &def) const
            {
                // Start with the string hash.
                size_t hash = std::hash<std::string>{}(def.fileName);

                // Apply some really quick and dirty change to the hash based
                // on the only other values that are different.
                hash ^= def.start;
                hash ^= def.length;

                return hash;
            }
        };

        /// Hash table of load requests so we can quickly get to them
        /// by name.
        std::unordered_map<LoadRequestDef, LoadRequest*, LoadRequestHash> loadRequestsByName;

        /// Set this to true when we want the loader thread to
        /// die. (This happens in the destructor for AssetLoader.)
        bool loaderThreadQuit;

        Threads::Thread *loaderThread;
        friend void AssetLoader_loaderThreadFunc(void *data);

        /// This is set to true every time the loader thread starts
        /// loading something. It's set to false every time the loader
        /// runs an iteration without doing anything. No locking is
        /// done on this.
        bool hasBeenLoading;
    };
}

#endif

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_THREADS

namespace ExPop
{
    inline void AssetLoader_loaderThreadFunc(void *data)
    {
        AssetLoader *loader = (AssetLoader*)data;

        while(!loader->loaderThreadQuit) {

            if(!loader->processLoadRequest()) {

                // If there was nothing on the queue, go to sleep for
                // a bit. It's only acceptable to run continuously if
                // it's still got stuff to do.
              #if _WIN32
                Sleep(1);
              #else
                usleep(1000);
              #endif
            }

        }

    }

    inline AssetLoader::AssetLoader(void)
    {
        hasBeenLoading = false;
        currentLoad = NULL;
        loaderThreadQuit = false;
        loaderThread = new Threads::Thread(AssetLoader_loaderThreadFunc, this);
    }

    inline AssetLoader::~AssetLoader(void)
    {
        // Clean up loader thread.
        loaderThreadQuit = true;
        loaderThread->join();
        delete loaderThread;

        // Clean up buffers.
        for(unsigned int i = 0; i < pendingLoads.size(); i++) {
            delete pendingLoads[i];
        }

        for(unsigned int i = 0; i < finishedLoads.size(); i++) {
            delete finishedLoads[i];
        }

        if(currentLoad) {
            delete currentLoad;
        }
    }

    inline bool AssetLoader::processLoadRequest(void)
    {
        loadListMutex.lock(); {

            if(pendingLoads.size()) {

                // Find the highest priority pending thing.
                float highestPriority = pendingLoads[0]->priority;
                int highestPriorityIndex = 0;
                for(unsigned int i = 1; i < pendingLoads.size(); i++) {
                    if(pendingLoads[i]->priority >= highestPriority) {
                        highestPriorityIndex = (int)i;
                        highestPriority = pendingLoads[i]->priority;
                    }
                }

                // Take the highest priority thing out of the list.
                currentLoad = pendingLoads[highestPriorityIndex];
                pendingLoads[highestPriorityIndex] = pendingLoads[pendingLoads.size() - 1];
                pendingLoads.erase(pendingLoads.end() - 1);

            }

        } loadListMutex.unlock();


        if(!currentLoad) {
            // Nothing to load.
            hasBeenLoading = false;
            return false;
        }

        // out("AssetLoader_thread") << "Loading file: " << currentLoad->fileName << endl;

        hasBeenLoading = true;
        currentLoad->started = true;

        // Actually load the file now.

        if(currentLoad->start == -1 || currentLoad->length == -1) {

            // Load the whole file.
            currentLoad->loadedBuffer = FileSystem::loadFile(
                currentLoad->fileName,
                &currentLoad->loadedBufferLength);

        } else {

            // out("AssetLoader_thread") << "Loading a slice: " << currentLoad->start << " " << currentLoad->length << endl;

            // Load a slice of the file.
            currentLoad->loadedBuffer = FileSystem::loadFilePart(
                currentLoad->fileName,
                currentLoad->length,
                currentLoad->start);

            currentLoad->loadedBufferLength = currentLoad->length;

        }

        // out("AssetLoader_thread") <<
        //     "Done loading: " << currentLoad->fileName <<
        //     " (" << (currentLoad->loadedBuffer ? "SUCCESS" : "FAIL") << ")" << endl;

        currentLoad->done = true;

        // Add it to the list of finished stuff.
        loadListMutex.lock(); {

            finishedLoads.push_back(currentLoad);
            currentLoad = NULL;

        } loadListMutex.unlock();

        return true;
    }

    inline bool AssetLoader::loading(void)
    {
        if(hasBeenLoading) {

            // Loader thread thinks it was doing something.
            return true;
        }

        // The loader thread hasn't told us it's loading something
        // yet, but it may be waiting because its queue was previously
        // empty, so check the list of stuff pending too.
        loadListMutex.lock();
        bool ret = pendingLoads.size() || currentLoad;
        loadListMutex.unlock();

        return ret;

    }

    inline AssetLoader::LoadRequest::LoadRequest(
        const std::string &fileName,
        float priority,
        int start,
        int length)
    {
        age = 0;
        done = false;
        started = false;
        loadedBuffer = NULL;
        loadedBufferLength = 0;
        this->priority = priority;
        this->fileName = fileName;
        this->start = start;
        this->length = length;
    }

    inline AssetLoader::LoadRequest::~LoadRequest(void)
    {
        delete[] loadedBuffer;
    }


    inline char *AssetLoader::requestData(
        const std::string &fileName,
        float priority,
        int *lengthOut,
        LoadStatus *status,
        int start,
        int length)
    {
        char *ret = NULL;
        LoadRequest *request = NULL;
        LoadRequestDef def(fileName, start, length);

        loadListMutex.lock(); {

            // First see if we're already loading this, or if it's in the
            // list of finished stuff.
            auto itr = loadRequestsByName.find(def);
            if(itr != loadRequestsByName.end()) {

                request = itr->second;

                // It already exists somewhere. Refresh the age regardless
                // of what state it's in.
                request->age = 0;

                // Raise priority if needed;
                if(request->priority < priority) {
                    request->priority = priority;
                }

                if(status) {
                    if(!request->started) {
                        *status = LOADSTATUS_WAITING;
                    } else if(!request->done) {
                        *status = LOADSTATUS_LOADING;
                    } else if(!request->loadedBuffer) {
                        *status = LOADSTATUS_FAIL;
                    } else {
                        *status = LOADSTATUS_SUCCESS;
                    }
                }

                if(lengthOut) {
                    *lengthOut = request->loadedBufferLength;
                }

                ret = request->loadedBuffer;

            } else {

                // It's not in the list yet. Create it and add it to the
                // hash table and the list of pending loads.
                request = new LoadRequest(fileName, priority, start, length);
                loadRequestsByName[def] = request;
                pendingLoads.push_back(request);
                if(status) {
                    *status = LOADSTATUS_WAITING;
                }

            }

        } loadListMutex.unlock();

        return ret;

    }

    inline void AssetLoader::ageData(int ageAmount)
    {
        loadListMutex.lock(); {

            for(unsigned int i = 0; i < finishedLoads.size(); i++) {

                finishedLoads[i]->age += ageAmount;

                // TODO: Replace this totally arbitrary age number
                // with something else.
                if(finishedLoads[i]->age > 30) {

                    // out("AssetLoader_thread") << "Clearing buffer for file: " << finishedLoads[i]->fileName <<
                    //     "(" << finishedLoads[i]->start <<
                    //     ", " << finishedLoads[i]->length << ")" << endl;

                    // This buffer is too old. Get rid of it.
                    loadRequestsByName.erase(
                        LoadRequestDef(
                            finishedLoads[i]->fileName,
                            finishedLoads[i]->start,
                            finishedLoads[i]->length));

                    delete finishedLoads[i];
                    finishedLoads[i] = finishedLoads[finishedLoads.size() - 1];
                    finishedLoads.erase(finishedLoads.end() - 1);
                    i--;
                }

            }

        } loadListMutex.unlock();
    }
}

#endif
