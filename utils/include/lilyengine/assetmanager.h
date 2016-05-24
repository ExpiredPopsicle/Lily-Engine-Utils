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
#include <iostream>

#include "assetloader.h"
#include "console.h"

namespace ExPop {

    class AssetLoader;

    /// AssetManager is a wrapper around a (possibly shared)
    /// AssetLoader for a specific kind of asset. It will handle
    /// instantiation of that asset and destruction of the asset after
    /// it hasn't been accessed for some amount of age.
    ///
    /// Example: Scripts loaded through the AssetManager might need to
    /// compile after loading and being placed in the list of
    /// available assets.
    ///
    /// Template parameter T is the asset type. E is the type for
    /// extra data to the instantiation function.
    ///
    /// Types used must have a creator function in the form of:
    ///   Object *func(
    ///          const std::string &name,
    ///          E extraLoadData,
    ///          void* data,
    ///          size_t dataLength);
    ///
    template<class T, class E>
    class AssetManager {
    public:

        // TODO: We really need better control over instantiation.
        // Instantiation function object?

        /// Create function type.
        typedef T *(*CreatorFunction)(
            const std::string &name, E loadData,
            void *data, size_t dataLength);

        AssetManager(AssetLoader *loader, E loadData, CreatorFunction creatorFunc);
        ~AssetManager(void);

        /// Increment the age of all assets by some amount. Anything
        /// older than the current maximum asset age will be deleted.
        void ageAssets(unsigned int age);

        /// Get an asset. AssetLoader status codes get passed through.
        /// Returns NULL if there's an error or the asset is not yet
        /// ready. This also refreshes the asset's age counter.
        /// Objects that the AssetLoader has fully loaded that have
        /// not yet been finished can be instantiated here, so beware
        /// initializations that might change other state (OpenGL
        /// textures, etc).
        T* getAsset(
            const std::string &name,
            AssetLoader::LoadStatus *status = NULL);

        /// Set the maximum age value before assets get automatically
        /// deleted.
        void setMaxAge(unsigned int maxAge);

        /// Get the maximum age value.
        unsigned int getMaxAge(void);

    private:

        CreatorFunction creatorFunc;

        class LoadedAsset {
        public:

            LoadedAsset(T *asset);
            ~LoadedAsset(void);

            T *asset;
            unsigned int age;
        };

        unsigned int maxAge;
        std::map<std::string, LoadedAsset*> loadedAssets;
        AssetLoader *loader;
        E extraLoadData;
    };



    template<class T, class E>
    AssetManager<T, E>::AssetManager(AssetLoader *loader, E loadData, CreatorFunction creatorFunc) :
        extraLoadData(loadData) {

        this->creatorFunc = creatorFunc;
        this->loader = loader;
        maxAge = 100;

        Console::outSetPrefix("AssetManager", "AssetManager: ");
    }

    template<class T, class E>
    AssetManager<T, E>::~AssetManager(void) {

        typename std::map<std::string, LoadedAsset*>::iterator i;
        for(i = loadedAssets.begin();
            i != loadedAssets.end(); i++) {
            delete (*i).second;
        }
    }

    template<class T, class E>
    void AssetManager<T, E>::ageAssets(unsigned int age) {

        std::vector<std::string> eraseList;

        typename std::map<std::string, LoadedAsset*>::iterator i;
        for(i = loadedAssets.begin();
            i != loadedAssets.end(); i++) {

            (*i).second->age += age;

            if((*i).second->age > maxAge) {
                eraseList.push_back((*i).first);
                delete (*i).second;
            }
        }

        for(unsigned int i = 0; i < eraseList.size(); i++) {
            Console::out("AssetManager") << "Purging asset: " << eraseList[i] << std::endl;
            loadedAssets.erase(eraseList[i]);
        }

    }

    template<class T, class E>
    void AssetManager<T, E>::setMaxAge(unsigned int maxAge) {
        this->maxAge = maxAge;
        ageAssets(0);
    }

    template<class T, class E>
    unsigned int AssetManager<T, E>::getMaxAge(void) {
        return maxAge;
    }


    template<class T, class E>
    T* AssetManager<T, E>::getAsset(
        const std::string &name,
        AssetLoader::LoadStatus *status) {

        if(loadedAssets.count(name)) {
            LoadedAsset *loadedAsset = loadedAssets[name];
            loadedAsset->age = 0;
            return loadedAsset->asset;
        }

        int length = 0;
        char *data = loader->requestData(name, 100, &length, status);

        if(data) {

            Console::out("AssetManager") << "Initializing asset: " << name << std::endl;

            T *newData = creatorFunc(
                name,
                extraLoadData,
                (void*)(data),
                size_t(length));

            LoadedAsset *newAsset = new LoadedAsset(newData);
            loadedAssets[name] = newAsset;

            return newData;
        }

        return NULL;
    }

    template<class T, class E>
    AssetManager<T, E>::LoadedAsset::LoadedAsset(T *asset) {
        this->asset = asset;
        age = 0;
    }

    template<class T, class E>
    AssetManager<T, E>::LoadedAsset::~LoadedAsset(void) {
        delete asset;
    }

};


