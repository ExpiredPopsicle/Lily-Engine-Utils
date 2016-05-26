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

#include <vector>
#include <iostream>
#include <cstring>
#include <cassert>
using namespace std;

#include "thread.h"

#if _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

// TODO: Add some idea of thread ownership. Like have the owner thread
// be the only one that can kill or join another thread or something.

// TODO: Add an option to start a thread in a suspended state.

namespace ExPop {

    namespace Threads {

        // -----------------------------------------------------------------------------
        //  Mutex class implementation.
        // -----------------------------------------------------------------------------

        struct Mutex::MutexPrivate {

          #if _WIN32

            // Windows mutexes.
            HANDLE winMutex;

          #else

            // Linux/Mac mutexes.
            pthread_mutex_t mutex;

          #endif

        };

        Mutex::Mutex(const Mutex &m) {
            mutexPrivate = new MutexPrivate();
            memcpy(mutexPrivate, m.mutexPrivate, sizeof(*mutexPrivate));
        }

        Mutex::Mutex(void) {

            mutexPrivate = new MutexPrivate();

          #if _WIN32

            // Windows
            mutexPrivate->winMutex = CreateMutex(0, FALSE, 0);

          #else

            // Linux/Mac
            pthread_mutex_init(&mutexPrivate->mutex, NULL);

          #endif

        }

        Mutex::~Mutex(void) {

          #if _WIN32

            // Windows
            CloseHandle(mutexPrivate->winMutex);

          #else

            // Linux/Mac
            pthread_mutex_destroy(&mutexPrivate->mutex);

          #endif

            delete mutexPrivate;
        }

        void Mutex::lock(void) {

          #if _WIN32

            // Windows
            WaitForSingleObject(mutexPrivate->winMutex, INFINITE);

          #else

            // Linux/Mac
            pthread_mutex_lock(&mutexPrivate->mutex);

          #endif

            lockingThread = getMyId();

        }

        void Mutex::unlock(void) {

          #if _WIN32

            // Windows
            ReleaseMutex(mutexPrivate->winMutex);

          #else

            // Linux/Mac
            pthread_mutex_unlock(&mutexPrivate->mutex);

          #endif

        }

        // -----------------------------------------------------------------------------
        //  Thread class implementation.
        // -----------------------------------------------------------------------------

        /// This is just the data needed to call the function.
        struct Thread::ThreadPrivate {
        public:
            void (*bareFunc)(void*);
            void *bareData;
        };


      #if _WIN32

        // Entry point for threads in Win32.
        DWORD __stdcall threadStarter(void *data) {
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

        // Entry point for threads in everywhere besides Win32.
        void *threadStarter(void *data) {
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

        Thread::Thread(void) {
            threadPrivate = NULL;
        }

        Thread::Thread(const Thread &t) {

            // threadPrivate may be invalid by this point.
            threadPrivate = t.threadPrivate;
            systemThread = t.systemThread;
            done = t.done;
        }

        Thread::Thread(void (*func)(void*), void *data) {

            assert(func);

            threadPrivate = new ThreadPrivate();
            threadPrivate->bareFunc = func;
            threadPrivate->bareData = data;

            finishInit();
        }

        void Thread::finishInit(void) {

            done = false;

          #if _WIN32

            // Windows

            // FIXME: Which version should I use?

            //systemThread = (HANDLE)_beginthreadex(0, 0, threadStarter, threadPrivate, CREATE_SUSPENDED, NULL);
            systemThread = (HANDLE)CreateThread(NULL, 0, threadStarter, threadPrivate, CREATE_SUSPENDED, NULL);

            ResumeThread(systemThread);

          #else

            // Linux/Mac
            pthread_create(&(systemThread), NULL,
                           threadStarter, threadPrivate);

          #endif

        }

        Thread::~Thread(void) {
            // Data isn't cleaned up here because it's cleaned up at the
            // end of threadStarter, which will only happen regardless of
            // how many Threads share a pointer to it.
        }

        void Thread::join(void) {

            if(!done) {

              #if _WIN32

                // Windows

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

              #else

                // Linux/Mac/Android
                pthread_join(systemThread, NULL);

              #endif
                done = true;

            } else {

                // This thread has already finished, apparently.

            }
        }

        ThreadId Thread::getId(void) {

            if(threadPrivate) {

              #if _WIN32

                // Windows
                return systemThread;

              #else

                // Linux/Mac
                return systemThread;

              #endif
            }

            return 0;

        }

        ThreadId getMyId(void) {

          #if _WIN32

            // Windows
            return GetCurrentThread();

          #else

            // Linux/Mac
            return pthread_self();

          #endif
        }

    }
}

