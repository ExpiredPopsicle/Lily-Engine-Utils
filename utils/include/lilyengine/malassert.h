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

#pragma once

#ifdef __linux__
#define exPopAssert(x) ExPop::fancyAssert((x), ##x)
#else
#define exPopAssert(x) assert(x)
#endif

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

