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

#include <iostream>
using namespace std;

#define EXPOP_ENABLE_TESTING 1
#include <lilyengine/utils.h>
using namespace ExPop;

#include "usagetext.h"

void showHelp(const char *argv0, bool error)
{
    std::ostream *out = error ? &std::cerr : &std::cout;
    *out << stringReplace<char>("$0", argv0, std::string(usageText, usageText_len)) << std::endl;
}

void showSectionHeader(const std::string &name)
{
    std::cout << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "  " << name << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> paramNames = { };
    std::vector<ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);

    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            showHelp(argv[0], false);
            return 0;
        } else {
            cerr << "Unrecognized parameter: " << params[i].name << endl;
            return 1;
        }
    }

    size_t passCounter = 0;
    size_t failCounter = 0;

    showSectionHeader("Testing system tests");
    EXPOP_TEST_VALUE(true, true);
    EXPOP_TEST_VALUE(false, false);

    showSectionHeader("Strings");
    doStringTests(passCounter, failCounter);

    showSectionHeader("Base64");
    doBase64Tests(passCounter, failCounter);

    showSectionHeader("Http");
    doHttpTests(passCounter, failCounter);

    showSectionHeader("AssetLoader");
    doAssetLoadTests(passCounter, failCounter);

    showSectionHeader("Angle");
    doAngleTests(passCounter, failCounter);

    showSectionHeader("Results");
    std::cout << "Passed: " << passCounter << std::endl;
    std::cout << "Failed: " << failCounter << std::endl;
    cout << std::endl;

    if(failCounter) {
        return 1;
    }

    return 0;
}


