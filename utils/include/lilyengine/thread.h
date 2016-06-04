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

// This thread implementation could be a lot more complete. Should
// probably add things like semaphores and condition variables. Maybe
// some day.

// Effectively, this system is just a wrapper around either Windows
// threads or pthreads, depending on what platform you're building on.

// This system is still needed for some other parts of
// Lily-Engine-Utils. Mainly this is because of MinGW having serious
// issues with std::thread for some reason. Otherwise we'd just rip it
// out and replace it all with std::thread.

// TODO: Add some idea of thread ownership. Like have the owner thread
// be the only one that can kill or join another thread or something.

// TODO: Add an option to start a thread in a suspended state.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "config.h"

#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>

#if _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_THREADS
namespace ExPop
{
    namespace Threads
    {

      #if _WIN32
        typedef DWORD ThreadId;
        typedef HANDLE ThreadType;
      #else
        typedef pthread_t ThreadId;
        typedef pthread_t ThreadType;
      #endif

        /// Get the current thread's ID.
        ThreadId getMyId(void);

        /// Simple mutex class.
        class Mutex
        {
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
        class Thread
        {
        public:

            Thread(void);
            ~Thread(void);

            /// Copy an existing thread handle.
            Thread(const Thread &t);

            /// Start a function in a new thread.
            Thread(void (*bareFunc)(void*), void *bareData);

            /// Block until the thread's function returns. Note: This
            /// function itself is not threadsafe. Only call once from
            /// a single thread.
            void join(void);

            /// Get the OS-specific ID for this thread.
            ThreadId getId(void);

        private:

            void finishInit(void);

            bool done;

            struct ThreadPrivate;
            ThreadPrivate *threadPrivate;
            ThreadType systemThread; // OS-level handle for the thread.

            // Entry point for threads.
          #if _WIN32
            friend DWORD __stdcall threadStarter(void *data);
          #else
            friend void *threadStarter(void *data);
          #endif

        };
    }
}
#endif

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

#if EXPOP_ENABLE_THREADS
namespace ExPop
{
    namespace Threads
    {
        // -----------------------------------------------------------------------------
        //  Mutex class implementation.
        // -----------------------------------------------------------------------------

        struct Mutex::MutexPrivate
        {
          #if _WIN32 // Windows mutexes.
            HANDLE winMutex;
          #else // Linux/Mac mutexes.
            pthread_mutex_t mutex;
          #endif
        };

        inline Mutex::Mutex(const Mutex &m)
        {
            mutexPrivate = new MutexPrivate();
            memcpy(mutexPrivate, m.mutexPrivate, sizeof(*mutexPrivate));
        }

        inline Mutex::Mutex(void)
        {
            mutexPrivate = new MutexPrivate();

          #if _WIN32 // Windows
            mutexPrivate->winMutex = CreateMutex(0, FALSE, 0);
          #else // Linux/Mac
            pthread_mutex_init(&mutexPrivate->mutex, NULL);
          #endif
        }

        inline Mutex::~Mutex(void)
        {
          #if _WIN32 // Windows
            CloseHandle(mutexPrivate->winMutex);
          #else // Linux/Mac
            pthread_mutex_destroy(&mutexPrivate->mutex);
          #endif

            delete mutexPrivate;
        }

        inline void Mutex::lock(void)
        {
          #if _WIN32 // Windows
            WaitForSingleObject(mutexPrivate->winMutex, INFINITE);
          #else // Linux/Mac
            pthread_mutex_lock(&mutexPrivate->mutex);
          #endif

            lockingThread = getMyId();
        }

        inline void Mutex::unlock(void)
        {
          #if _WIN32 // Windows
            ReleaseMutex(mutexPrivate->winMutex);
          #else // Linux/Mac
            pthread_mutex_unlock(&mutexPrivate->mutex);
          #endif
        }

        // -----------------------------------------------------------------------------
        //  Thread class implementation.
        // -----------------------------------------------------------------------------

        /// This is just the data needed to call the function.
        struct Thread::ThreadPrivate
        {
        public:
            void (*bareFunc)(void*);
            void *bareData;
        };


      #if _WIN32

        // Entry point for threads in Windows.
        inline DWORD __stdcall threadStarter(void *data)
        {
            Thread::ThreadPrivate *threadPrivate = (Thread::ThreadPrivate*)data;
            if(threadPrivate) {
                if(threadPrivate->bareFunc) {
                    threadPrivate->bareFunc(threadPrivate->bareData);
                }
                delete threadPrivate;
            }
            return 0;
        }

      #else

        // Entry point for threads in everywhere besides Windows.
        inline void *threadStarter(void *data)
        {
            Thread::ThreadPrivate *threadPrivate = (Thread::ThreadPrivate*)data;
            if(threadPrivate) {
                if(threadPrivate->bareFunc) {
                    threadPrivate->bareFunc(threadPrivate->bareData);
                }
                delete threadPrivate;
            }
            return NULL;
        }

      #endif

        inline Thread::Thread(void)
        {
            threadPrivate = NULL;
        }

        inline Thread::Thread(const Thread &t)
        {
            // threadPrivate may be invalid by this point.
            threadPrivate = t.threadPrivate;
            systemThread = t.systemThread;
            done = t.done;
        }

        inline Thread::Thread(void (*func)(void*), void *data)
        {
            assert(func);

            threadPrivate = new ThreadPrivate();
            threadPrivate->bareFunc = func;
            threadPrivate->bareData = data;

            finishInit();
        }

        inline void Thread::finishInit(void)
        {
            done = false;

          #if _WIN32 // Windows

            // FIXME: Which version should I use?

            //systemThread = (HANDLE)_beginthreadex(0, 0, threadStarter, threadPrivate, CREATE_SUSPENDED, NULL);
            systemThread = (HANDLE)CreateThread(NULL, 0, threadStarter, threadPrivate, CREATE_SUSPENDED, NULL);

            ResumeThread(systemThread);

          #else // Linux/Mac
            pthread_create(&(systemThread), NULL,
                           threadStarter, threadPrivate);

          #endif
        }

        inline Thread::~Thread(void)
        {
            // Data isn't cleaned up here because it's cleaned up at
            // the end of threadStarter, which will happen regardless
            // of how many Threads share a pointer to it.
        }

        inline void Thread::join(void)
        {
            if(!done) {

              #if _WIN32 // Windows

                DWORD ret = WaitForSingleObject(systemThread, INFINITE);

                // FIXME: This is really just a placeholder with the
                // possible return values in case I need to go back and
                // add some real error handling later.
                switch(ret) {
                    case WAIT_ABANDONED:
                    case WAIT_TIMEOUT:
                    case WAIT_FAILED:
                        // These are all failures, but I'm not sure what
                        // we should do about it.
                        break;

                    case WAIT_OBJECT_0:
                        // Success, I think.
                        break;

                    default:
                        // Uhhhh...
                        break;
                }

                // FIXME: If multiple threads are trying to join() to this
                // thread, calling this multiple times will be bad.
                CloseHandle(systemThread);

              #else // Linux/Mac/Android

                pthread_join(systemThread, NULL);

              #endif
                done = true;

            } else {

                // This thread has already finished, apparently.

            }
        }

        inline ThreadId Thread::getId(void)
        {
            if(threadPrivate) {
              #if _WIN32 // Windows
                return GetThreadId(systemThread);
              #else // Linux/Mac
                return systemThread;
              #endif
            }
            return 0;
        }

        inline ThreadId getMyId(void)
        {
          #if _WIN32 // Windows
            return GetCurrentThreadId();
          #else // Linux/Mac
            return pthread_self();
          #endif
        }
    }
}
#endif

