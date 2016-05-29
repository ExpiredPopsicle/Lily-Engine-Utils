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

// This is a simple assertion system that, on Linux, will dump a call
// stack to stdout on failure before raising the SIGINT. Sometimes
// saves a little tiny bit of time from having to open the debugger to
// deal with an assert.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <cassert>
#include <iostream>

#if __linux__
#include <cstdlib>
#include <malloc.h>
#include <execinfo.h>
#include <signal.h>
#include <iomanip>
#endif

#ifdef __linux__
#define exPopAssert(x) ExPop::fancyAssert((x), ##x)
#else
#define exPopAssert(x) assert(x)
#endif

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop {

    /// If on Linux, spew out a stack trace to the console and raise
    /// SIGINT so we can debug. On other platforms just assert(0)
    /// after showing the failed assertion.
    void fancyAssertFail(const char *errMsg);

    /// Assert with a custom error message. Use exPopAssert(condition)
    /// to do with with just the condition as the message.
    inline void fancyAssert(bool b, const char *str)
    {
        if(b) return;
        fancyAssertFail(str);
    }

}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop {

    inline void fancyAssertFail(const char *errMsg)
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
};

