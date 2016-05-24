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

#include <cassert>
#include <iostream>

#if __linux__
#include <cstdlib>
#include <malloc.h>
#include <execinfo.h>
#include <signal.h>
#include <iomanip>
#endif

using namespace std;

namespace ExPop {

    void fancyAssertFail(const char *errMsg)
    {

#if __linux__

        // On Linux we may get access to a stack trace that we can
        // just dump to a console instead of having to load it up in
        // the debugger. Probaby only useful if we compiled with
        // debugging symbols enabled.

        const int stackLength = 256;
        void *stackArray[stackLength];
        char **symbols;
        size_t size;

        size = backtrace(stackArray, stackLength);
        symbols = backtrace_symbols(stackArray, size);
        cout << "Assertion \"" << errMsg << "\"failed. Here's the stack:" << endl;

        for(unsigned int i = 0; i < size && symbols; i++) {
            cout << setw(8) << i << ": " <<  symbols[i] << endl;
        }

        // I'm not sure if we should bother with a free() here,
        // because in the case of a heap corruption related failure,
        // this could make the situation even worse. We could also
        // just want to resume in the debugger after the SIGINT, in
        // which case we'd want heap allocations to go away.
        free(symbols);

        // Drop us into the debugger or just explode.
        raise(SIGINT);

#else

        // We probably won't hit this, because on non-Linux platforms
        // we just assert() normally.
        cout << "Assertion \"" << errMsg << "\"failed." << endl;
        assert(0);

#endif

    }

}
