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
    /// Types used must have a constructor in the form of:
    ///   Object(const std::string &name,
    ///          void* data,
    ///          size_t dataLength,
    ///          E extraLoadData);
    ///
    /// If I ever switch over to C++11 support, the interface to this
    /// will change.
    template<class T, class E>
    class AssetManager {
    public:

        // TODO: We really need better control over instantiation.
        // Instantiation function object?

        AssetManager(AssetLoader *loader, E loadData);
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
    AssetManager<T, E>::AssetManager(AssetLoader *loader, E loadData) :
        extraLoadData(loadData) {

        this->loader = loader;
        maxAge = 100;
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

            T *newData = new T(
                name,
                (void*)data,
                size_t(length),
                extraLoadData);

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


