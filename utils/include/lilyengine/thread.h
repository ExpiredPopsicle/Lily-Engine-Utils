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

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// This thread implementation could be a lot more complete. Should
// probably add things like semaphores and condition variables.

namespace ExPop {

    namespace Threads {

#ifdef WIN32
        typedef HANDLE ThreadId;
        typedef HANDLE ThreadType;
#else
        typedef pthread_t ThreadId;
        typedef pthread_t ThreadType;
#endif

        /// Get the current thread's ID.
        ThreadId getMyId(void);

        /// Simple mutex class.
        class Mutex {
        public:

            Mutex(const Mutex &m);
            Mutex(void);
            ~Mutex(void);

            /// Lock this. Blocks until it's available.
            void lock(void);

            /// Unlock this.
            void unlock(void);

        private:

            struct MutexPrivate;
            MutexPrivate *mutexPrivate;

            /// Keep track of whatever thread is locking this one.
            ThreadId lockingThread;
        };

        /// Handle to a thread. More than one of these objects can
        /// represent a single thread at a time.
        class Thread {
        public:

            Thread(void);
            ~Thread(void);

            /// Copy an existing thread handle.
            Thread(const Thread &t);

            /// Start a function in a new thread.
            Thread(void (*bareFunc)(void*), void *bareData);

            /// Block until the thread's function returns.
            void join(void);

            /// Get the OS-specific ID for this thread.
            ThreadId getId(void);

        private:

            void finishInit(void);

            bool done;

            struct ThreadPrivate;
            ThreadPrivate *threadPrivate;
            ThreadType systemThread;

            // Entry point for threads.
#ifdef WIN32
            friend DWORD __stdcall threadStarter(void *data);
#else
            friend void *threadStarter(void *data);
#endif

        };

    }

}

