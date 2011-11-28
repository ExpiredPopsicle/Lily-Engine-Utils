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

#ifndef EXPOP_REFSYSTEM_H
#define EXPOP_REFSYSTEM_H

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
        Ref(void) {

            this->ptr = NULL;

            refCount = new int;
            *refCount = 0;

            incRefCount();

            // Console::out("refsystem") << "Made NULL reference based on normal constructor." << std::endl;

        }

        /// Use this constructor mainly. Takes ownership of whatever
        /// you're pointing it to.
        Ref(T* ptr) {

            this->ptr = ptr;

            refCount = new int;
            *refCount = 0;

            incRefCount();

            // Console::out("refsystem") << "Made new reference to " << this->ptr << " based on normal constructor." << std::endl;

        }

        /// Copy constructor. Needed to define it just so we can
        /// increase the ref count automatically.
        Ref(const Ref<T> &otherRef) {

            this->refCount = otherRef.refCount;
            this->ptr = otherRef.ptr;

            incRefCount();

            // Console::out("refsystem") << "Made new reference to " << this->ptr << " based on copy constructor." << std::endl;

        }

        /// Assignment operator. Decrements ref count to whatever we
        /// were pointing to before and increments it for the new
        /// thing.
        Ref &operator=(const Ref<T> otherRef) {

            // Console::out("refsystem") << "Using operator= to go from " << this->ptr << " to " << otherRef.ptr << std::endl;

            decRefCount();

            this->refCount = otherRef.refCount;
            this->ptr = otherRef.ptr;

            incRefCount();

            return *this;
        }

        ~Ref(void) {

            // Console::out("refsystem") << "Cleaning up a reference to " << this->ptr << std::endl;

            decRefCount();
        }

        inline int getRefCount(void) {
            if(refCount) {
                return *refCount;
            }
        }

        /// This is the important part. Get the original pointer back
        /// out of the ref system.
        inline T* getPtr(void) {
            return ptr;
        }

        inline T* operator->(void) {
            assert(ptr);
            return ptr;
        }

        inline T& operator*(void) {
            assert(ptr);
            return *ptr;
        }

    private:

        T *ptr;
        int *refCount;

        // Increment refcount.
        inline void incRefCount(void) {
            if(refCount) {
                (*refCount)++;
                // Console::out("refsystem") << "Incremented. RefCount for " << this->ptr << " is now " << *refCount << std::endl;
            }
        }

        // Decrement refcount (and clean up if we hit zero).
        inline void decRefCount(void) {
            if(refCount) {
                (*refCount)--;
                // Console::out("refsystem") << "Decremented. RefCount for " << this->ptr << " is now " << *refCount << std::endl;

                if(!*refCount) {
                    // Clean up object and refCount.
                    delete refCount;
                    delete ptr;
                    // Console::out("refsystem") << "RefCount for " << this->ptr << " hit zero. Deleted it." << std::endl;
                    refCount = NULL;
                    ptr = NULL;
                }
            }
        }


    };

};

#endif
