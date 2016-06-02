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

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <chrono>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

#include "usagetext.h"

// ----------------------------------------------------------------------
// #defines needed to do all the tests
// ----------------------------------------------------------------------

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

// ----------------------------------------------------------------------
// Actual tests
// ----------------------------------------------------------------------

inline void doAngleTests(size_t &passCounter, size_t &failCounter)
{
    // Comparing floats for equality like this is generally a bad
    // idea, but I think it'll be okay here.
    EXPOP_TEST_VALUE(Angle(360.0f).getDegrees(), 0.0f);
    EXPOP_TEST_VALUE(Angle(361.0f).getDegrees(), 1.0f);
    EXPOP_TEST_VALUE(Angle(-1.0f).getDegrees(), 359.0f);
    EXPOP_TEST_VALUE(Angle(-361.0f).getDegrees(), 359.0f);
    EXPOP_TEST_VALUE(interpAngle(Angle(-360.0f), Angle(1.0f), 0.5f).getDegrees(), 0.5f);
}

inline void doAssetLoadTests(size_t &passCounter, size_t &failCounter)
{
    AssetLoader loader;

    int length = 0;
    char *data = nullptr;
    std::cout << "Loading the readme" << std::endl;

    size_t loopCount = 0;

    while(true) {
        data = loader.requestData("README.org", 100, &length);
        if(data) {
            break;
        }

        // Show a progress bar thingy.
        if(loopCount % 70 == 0 && loopCount) {
            std::cout << std::endl;
        }
        std::cout << ".oO0Oo"[loopCount % 6];
        loopCount++;
    }
    std::cout << std::endl;

    EXPOP_TEST_VALUE(!!data, true);
    EXPOP_TEST_VALUE(std::string(data, length), FileSystem::loadFileString("README.org"));
}

inline void doBase64Tests(size_t &passCounter, size_t &failCounter)
{
    EXPOP_TEST_VALUE(stringBase64EncodeString("butts"), "YnV0dHM=");
    EXPOP_TEST_VALUE(stringBase64EncodeString("ASDFGBC"), "QVNERkdCQw==");
    EXPOP_TEST_VALUE(stringBase64EncodeString(std::string("\0\0\0\0\0\0\0\0", 8)), "AAAAAAAAAAA=");
}

inline void doHttpTests(size_t &passCounter, size_t &failCounter)
{
    EXPOP_TEST_VALUE(httpGet("http://butts@expiredpopsicle.com:80/index.html").success, true);
    EXPOP_TEST_VALUE(httpGet("http://expiredpopsicle.com").success, true);
}

inline void doStringTests(size_t &passCounter, size_t &failCounter)
{
    EXPOP_TEST_VALUE(stringStartsWith<char>("dick", "dickbutts"), true);
    EXPOP_TEST_VALUE(stringEndsWith<char>("butts", "dickbutts"), true);
    EXPOP_TEST_VALUE(!stringEndsWith<char>("dick", "dickbutts"), true);
    EXPOP_TEST_VALUE(!stringStartsWith<char>("butts", "dickbutts"), true);
    EXPOP_TEST_VALUE(stringUnescape<char>("foo\\n\\\\"), "foo\n\\");
    EXPOP_TEST_VALUE("foo\\n\\\\", stringEscape<char>("foo\n\\"));
    EXPOP_TEST_VALUE(stringTrim<char>("  asdf"), std::string("asdf"));
    EXPOP_TEST_VALUE(stringTrim<char>("asdf  "), std::string("asdf"));
    EXPOP_TEST_VALUE(stringTrim<char>("  asdf  "), std::string("asdf"));
    EXPOP_TEST_VALUE(stringTrim<char>("asdf"), std::string("asdf"));
    EXPOP_TEST_VALUE(stringTrim<char>("  "), std::string(""));
    EXPOP_TEST_VALUE(stringTrim<char>(""), std::string(""));
    EXPOP_TEST_VALUE(stringReplace<char>("BUTT", "BOOB", "DICKBUTT"), "DICKBOOB");
    EXPOP_TEST_VALUE(stringReplace<char>("DICK", "BOOB", "DICKBUTT"), "BOOBBUTT");
    EXPOP_TEST_VALUE(stringReplace<char>("DICKBUTT", "BOOBS", "DICKBUTT"), "BOOBS");
    EXPOP_TEST_VALUE(stringReplace<char>("DICKBUTTASDF", "BOOBS", "DICKBUTT"), "DICKBUTT");
}

class TimerBlock
{
public:
    TimerBlock(const char *inName)
    {
        startTime = std::chrono::high_resolution_clock::now();
        name = inName;
    }

    ~TimerBlock()
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds t = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        std::cout << name << ": " << t.count() << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    const char *name;
};

#define TIME_SECTION(name) TimerBlock timerBlock(name)

inline void doHashTableTests(size_t &passCounter, size_t &failCounter)
{
    // std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
    // // Assume I did something cool here.
    // std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
    // std::chrono::nanoseconds t = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    // std::cout << t.count() << std::endl;

    {
        TIME_SECTION("std::map        ");
        std::map<unsigned int, unsigned int> theMap;
        srand(0);
        for(size_t i = 0; i < 16 * 1024 * 1024; i++) {
            theMap[rand() % 2048] = rand();
        }
    }

    {
        TIME_SECTION("std::unorded_map");
        std::unordered_map<unsigned int, unsigned int> theMap;
        srand(0);
        for(size_t i = 0; i < 16 * 1024 * 1024; i++) {
            theMap[rand() % 2048] = rand();
        }
    }

    {
        TIME_SECTION("ExPop::HashTable");
        ExPop::HashTable<unsigned int, unsigned int> theMap;
        srand(0);
        for(size_t i = 0; i < 16 * 1024 * 1024; i++) {
            theMap[rand() % 2048] = rand();
        }
    }
}

// ----------------------------------------------------------------------
// Testing 'framework'
// ----------------------------------------------------------------------

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

    showSectionHeader("HashTable");
    doHashTableTests(passCounter, failCounter);

    showSectionHeader("Results");
    std::cout << "Passed: " << passCounter << std::endl;
    std::cout << "Failed: " << failCounter << std::endl;
    cout << std::endl;

    if(failCounter) {
        return 1;
    }

    return 0;
}


