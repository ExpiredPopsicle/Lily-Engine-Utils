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

// Internal testing stuff for Lily Engine Utils.

#pragma once

#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>

#if _WIN32
// Windows console doesn't like VT100 color codes.
#define EXPOP_TEST_COLOR_RESET ""
#define EXPOP_TEST_COLOR_GOOD  ""
#define EXPOP_TEST_COLOR_BAD   ""
#define EXPOP_TEST_COLOR_NOTICE ""
#else
#define EXPOP_TEST_COLOR_RESET "\033[0m"
#define EXPOP_TEST_COLOR_GOOD  "\033[1;32m"
#define EXPOP_TEST_COLOR_BAD   "\033[1;31;5m"
#define EXPOP_TEST_COLOR_NOTICE "\033[1;36m"
#endif

#define EXPOP_TEST_VALUE(x, y)                                      \
    do {                                                            \
        bool pass = ((x) == (y));                                   \
        std::ostringstream testingOstr;                             \
        testingOstr << "" << #x << " == " << #y << " ";             \
        std::string testingStr = testingOstr.str();                 \
        if(testingStr.size() > 70)                                  \
            testingStr = testingStr.substr(0, 70) + "...";          \
        testingStr = testingStr + " ";                              \
        std::cout << std::setw(80-4) << std::left << testingStr;    \
        if(pass) {                                                  \
            std::cout << EXPOP_TEST_COLOR_GOOD;                     \
            std::cout << "PASS" << std::endl;                       \
            std::cout << EXPOP_TEST_COLOR_RESET;                    \
            passCounter++;                                          \
        } else {                                                    \
            std::cout << EXPOP_TEST_COLOR_BAD;                      \
            std::cout << "FAIL (" << (x) <<  ")" << std::endl;      \
            std::cout << EXPOP_TEST_COLOR_RESET;                    \
            failCounter++;                                          \
        }                                                           \
    } while(0)
