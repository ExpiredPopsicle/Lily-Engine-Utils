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

#include <lilyengine/config.h>

#if EXPOP_ENABLE_THREADS

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#if !_WIN32
#include <unistd.h>
#endif

#include <lilyengine/winhacks.h>
#include <lilyengine/filesystem.h>
#include <lilyengine/assetloader.h>

#if EXPOP_CONSOLE
#include <lilyengine/console.h>
using namespace ExPop::Console;
#endif

namespace ExPop
{
    void AssetLoader_loaderThreadFunc(void *data)
    {
        AssetLoader *loader = (AssetLoader*)data;

        while(!loader->loaderThreadQuit) {

            if(!loader->processLoadRequest()) {

                // If there was nothing on the queue, go to sleep for
                // a bit. It's only acceptable to run continuously if
                // it's still got stuff to do.
                usleep(1000);
            }

        }

    }

    AssetLoader::AssetLoader(void)
    {
        hasBeenLoading = false;
        currentLoad = NULL;
        loaderThreadQuit = false;
        loaderThread = new Threads::Thread(AssetLoader_loaderThreadFunc, this);

      #if EXPOP_CONSOLE
        outSetPrefix("AssetLoader_thread", "AssetLoader(thread): ");
      #endif
    }

    AssetLoader::~AssetLoader(void)
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

    bool AssetLoader::processLoadRequest(void)
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

      #if EXPOP_CONSOLE
        out("AssetLoader_thread") << "Loading file: " << currentLoad->fileName << endl;
      #endif

        hasBeenLoading = true;
        currentLoad->started = true;

        // Actually load the file now.

        if(currentLoad->start == -1 || currentLoad->length == -1) {

            // Load the whole file.
            currentLoad->loadedBuffer = FileSystem::loadFile(
                currentLoad->fileName,
                &currentLoad->loadedBufferLength);

        } else {

          #if EXPOP_CONSOLE
            out("AssetLoader_thread") << "Loading a slice: " << currentLoad->start << " " << currentLoad->length << endl;
          #endif

            // Load a slice of the file.
            currentLoad->loadedBuffer = FileSystem::loadFilePart(
                currentLoad->fileName,
                currentLoad->length,
                currentLoad->start);

            currentLoad->loadedBufferLength = currentLoad->length;

        }

      #if EXPOP_CONSOLE
        out("AssetLoader_thread") <<
            "Done loading: " << currentLoad->fileName <<
            " (" << (currentLoad->loadedBuffer ? "SUCCESS" : "FAIL") << ")" << endl;
      #endif

        currentLoad->done = true;

        // Add it to the list of finished stuff.
        loadListMutex.lock(); {

            finishedLoads.push_back(currentLoad);
            currentLoad = NULL;

        } loadListMutex.unlock();

        return true;
    }

    bool AssetLoader::loading(void)
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

    AssetLoader::LoadRequest::LoadRequest(
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

    AssetLoader::LoadRequest::~LoadRequest(void)
    {
        delete[] loadedBuffer;
    }


    char *AssetLoader::requestData(
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

    void AssetLoader::ageData(int ageAmount)
    {
        loadListMutex.lock(); {

            for(unsigned int i = 0; i < finishedLoads.size(); i++) {

                finishedLoads[i]->age += ageAmount;

                // TODO: Replace this totally arbitrary age number
                // with something else.
                if(finishedLoads[i]->age > 30) {

                  #if EXPOP_CONSOLE
                    out("AssetLoader_thread") << "Clearing buffer for file: " << finishedLoads[i]->fileName <<
                        "(" << finishedLoads[i]->start <<
                        ", " << finishedLoads[i]->length << ")" << endl;
                  #endif

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

  #undef out
};

#endif
