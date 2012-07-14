// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2012 Clifford Jolly
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

#include <string>
#include <vector>

#include "../hashtable/hashtable.h"

namespace ExPop {

    class Thread;

    /// Background asset loading system.
    class AssetLoader {
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
            int loadedBufferLength;
            int age;

        };

        /// We use these for hashing load requests.
        class LoadRequestDef {
        public:
            std::string fileName;
            int start;
            int length;

            inline LoadRequestDef(void) {
                start = -1;
                length = -1;
            }

            inline LoadRequestDef(
                std::string fileName,
                int start,
                int length) {

                this->fileName = fileName;
                this->start = start;
                this->length = length;
            }

            inline bool operator==(const LoadRequestDef &otherDef) {
                return
                    start == otherDef.start &&
                    length == otherDef.length &&
                    fileName == otherDef.fileName;
            }

            inline bool operator<(const LoadRequestDef &otherDef) {

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

        /// Hash table of load requests so we can quickly get to them
        /// by name.
        HashTable<LoadRequestDef, LoadRequest*> loadRequestsByName;

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

        friend HashValue genericHashFunc(const AssetLoader::LoadRequestDef &def);
    };

}


