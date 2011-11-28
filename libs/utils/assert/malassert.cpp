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

    void fancyAssertFail(const char *errMsg) {

#if __linux__

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

        free(symbols);

        // Drop us into the debugger or just explode.
        raise(SIGINT);

#else

        cout << "Assertion \"" << errMsg << "\"failed." << endl;
        assert(0);

#endif

    }

}
