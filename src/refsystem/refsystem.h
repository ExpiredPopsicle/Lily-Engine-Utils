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

#ifndef EXPOP_REFSYSTEM_H
#define EXPOP_REFSYSTEM_H

#include <cassert>
#include <map>
#include <iostream>
#include "../console/console.h"

namespace ExPop {

    // NOTE: Please excuse all the Console debugging crap strewn about
    // this file. I would normally disable that output but Win32 seems
    // to suffer significant overhead even if that is the case.

    /// Simple reference system. This class wraps a pointer. When no
    /// instances of this class exist that point to the given pointer,
    /// it'll free whatever it was pointing to. So whatever you toss
    /// into the reference system gets owned by the reference
    /// system. Also, this is not thread safe at all.
    template<class T>
    class Ref {
    public:

        /// Default constructor is pretty much just a reference to
        /// NULL.
        Ref(void);

        /// Use this constructor mainly. Takes ownership of whatever
        /// you're pointing it to.
        Ref(T* ptr);

        /// Copy constructor. Needed to define it just so we can
        /// increase the ref count automatically.
        Ref(const Ref<T> &otherRef);

        ~Ref(void);

        /// Make a weak reference. The reference returned from this
        /// will not count towards reference counts and will be set to
        /// NULL when all strong references are gone.
        Ref<T> makeWeakRef(void);

        /// Make a strong (default type) reference. This can be done
        /// from a weak reference.
        Ref<T> makeStrongRef(void);

        /// Assignment operator. Decrements ref count to whatever we
        /// were pointing to before and increments it for the new
        /// thing.
        Ref<T> &operator=(const Ref<T> &otherRef);

        /// Get the reference count. Returns zero if this is a NULL
        /// pointer.
        inline int getRefCount(void);

        /// This is the important part. Get the original pointer back
        /// out of the ref system.
        inline T* getPtr(void);

        /// Indirection operator.
        inline T* operator->(void);

        /// Dereference operator.
        inline T& operator*(void);

        /// This is so we can use this with hash maps and std::map.
        inline bool operator<(const Ref<T> &otherRef) const;

    private:

        /// Internal data shared among all references to this one
        /// instance of whatever.
        struct InternalData {
            T *ptr;
            int *refCount;
            Ref<T> *weakReferencesList;
        };

        /// One internal data is shared by all references to the same
        /// object.
        InternalData *internalData;

        /// This is true if this is a weak reference. Beware of
        /// getting into invalid states by messing with this.
        bool weakRef;

        /// Pointer to the next weak reference (if this is a weak
        /// reference).
        Ref<T> *nextWeakReference;

        /// Backpointer to the last pointer to this from the previous
        /// entry in the list.
        Ref<T> **lastWeakReferenceBackPtr;

        /// Increment refcount.
        inline void incRefCount(void);

        /// Decrement refcount (and clean up if we hit zero).
        inline void decRefCount(void);

        /// Remove this from the weak references list.
        void removeFromWeakReferences(void);

        /// Add this to the weak references list.
        void addToWeakReferences(void);

    };


    // Implementation follows...
    // ----------------------------------------------------------------------

    template<class T>
    Ref<T>::Ref(void) {

        internalData = NULL;
        weakRef = false;
        nextWeakReference = NULL;
        lastWeakReferenceBackPtr = NULL;

    }

    template<class T>
    Ref<T>::Ref(T* ptr) {

        weakRef = false;
        nextWeakReference = NULL;
        lastWeakReferenceBackPtr = NULL;

        if(ptr) {

            internalData = new InternalData;

            internalData->refCount = 0;
            internalData->ptr = ptr;
            internalData->weakReferencesList = NULL;

            incRefCount();

        } else {

            internalData = NULL;

        }

        // Console::out("refsystem") << "Made new reference to " << this->ptr << " based on normal constructor." << std::endl;

    }

    /// Copy constructor. Needed to define it just so we can
    /// increase the ref count automatically.
    template<class T>
    Ref<T>::Ref(const Ref<T> &otherRef) {

        weakRef = false;
        nextWeakReference = NULL;
        lastWeakReferenceBackPtr = NULL;

        this->internalData = otherRef.internalData;
        this->weakRef = otherRef.weakRef;

        addToWeakReferences();
        incRefCount();

        // Console::out("refsystem") << "Made new reference to " << this->ptr << " based on copy constructor." << std::endl;

    }

    template<class T>
    Ref<T> Ref<T>::makeWeakRef(void) {

        Ref<T> newRef;
        newRef.internalData = internalData;
        newRef.weakRef = true;
        newRef.addToWeakReferences();
        return newRef;
    }

    template<class T>
    Ref<T> Ref<T>::makeStrongRef(void) {

        Ref<T> newRef;
        newRef.internalData = internalData;
        newRef.incRefCount();
        return newRef;

    }

    /// Assignment operator. Decrements ref count to whatever we
    /// were pointing to before and increments it for the new
    /// thing.
    template<class T>
    Ref<T> &Ref<T>::operator=(const Ref<T> &otherRef) {

        // Console::out("refsystem") << "Using operator= to go from " << this->ptr << " to " << otherRef.ptr << std::endl;

        removeFromWeakReferences();
        decRefCount();

        this->internalData = otherRef.internalData;
        this->weakRef = otherRef.weakRef;

        addToWeakReferences();
        incRefCount();

        return *this;
    }

    template<class T>
    Ref<T>::~Ref(void) {

        removeFromWeakReferences();

        // Console::out("refsystem") << "Cleaning up a reference to " << this->ptr << std::endl;

        decRefCount();
    }

    template<class T>
    inline int Ref<T>::getRefCount(void) {
        if(internalData) {
            return internalData->refCount;
        }

        // FIXME: Default return for a NULL pointer. I don't know
        // if this should be zero or one (indicating this one
        // reference).
        return 0;
    }

    /// This is the important part. Get the original pointer back
    /// out of the ref system.
    template<class T>
    inline T* Ref<T>::getPtr(void) {
        if(!internalData) return NULL;
        return internalData->ptr;
    }

    template<class T>
    inline T* Ref<T>::operator->(void) {
        assert(internalData);
        return internalData->ptr;
    }

    template<class T>
    inline T& Ref<T>::operator*(void) {
        assert(internalData);
        return *internalData->ptr;
    }

    template<class T>
    inline bool Ref<T>::operator<(const Ref<T> &otherRef) const {
        return internalData->ptr < otherRef.internalData->ptr;
    }

    template<class T>
    inline void Ref<T>::incRefCount(void) {

        if(weakRef) return;

        if(internalData) {
            internalData->refCount++;
            // Console::out("refsystem") << "Incremented. RefCount for " << this->ptr << " is now " << *refCount << std::endl;
        }
    }

    template<class T>
    inline void Ref<T>::decRefCount(void) {

        if(weakRef) return;

        if(internalData) {
            internalData->refCount--;
            // Console::out("refsystem") << "Decremented. RefCount for " << this->ptr << " is now " << *refCount << std::endl;

            if(!internalData->refCount) {

                // This is the last reference to this. Clean up
                // everything.

                // Clean up all weak references.
                while(internalData->weakReferencesList) {
                    Ref<T> *weakPtr = internalData->weakReferencesList;
                    weakPtr->removeFromWeakReferences();
                    weakPtr->internalData = NULL;
                }

                // Clean up object and refCount.
                delete internalData->ptr;
                delete internalData;

                internalData = NULL;
            }
        }
    }

    template<class T>
    void Ref<T>::removeFromWeakReferences(void) {

        if(!weakRef) return;

        // Console::out("refsystem") << "Removing " << this << " from weak references" << endl;

        if(lastWeakReferenceBackPtr) {
            *lastWeakReferenceBackPtr = nextWeakReference;

            if(nextWeakReference) {
                nextWeakReference->lastWeakReferenceBackPtr = lastWeakReferenceBackPtr;
            }
        }

        nextWeakReference = NULL;
        lastWeakReferenceBackPtr = NULL;
    }

    template<class T>
    void Ref<T>::addToWeakReferences(void) {

        if(!weakRef) return;

        // Console::out("refsystem") << "Adding " << this << " to weak references" << endl;

        assert(!lastWeakReferenceBackPtr);
        assert(!nextWeakReference);

        // Insert into front of weak references list.
        lastWeakReferenceBackPtr = &internalData->weakReferencesList;
        nextWeakReference = internalData->weakReferencesList;
        if(nextWeakReference) {
            nextWeakReference->lastWeakReferenceBackPtr = &(this->nextWeakReference);
        }
        internalData->weakReferencesList = this;
    }

};

#endif
