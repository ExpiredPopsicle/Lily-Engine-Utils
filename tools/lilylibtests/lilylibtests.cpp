// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
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

#if !_WIN32
#include <unistd.h>
#endif
#include <lilyengine/winhacks.h>
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
    EXPOP_TEST_VALUE(stringBase64EncodeString(std::string("herpy derpy derp")), "aGVycHkgZGVycHkgZGVycA==");
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

inline void doRC4Tests(size_t &passCounter, size_t &failCounter)
{
    {
        // This is a small example from the CipherSaber page, but
        // split up so we have the initialization vector already as
        // part of the key.
        static const unsigned char cstest1[] = {
            'a', 's', 'd', 'f', 'g',
            0x6f, 0x6d, 0x0b, 0xab, 0xf3, 0xaa, 0x67, 0x19, 0x03, 0x15,
        };

        // And the actual ciphertext.
        unsigned char cstest2[] = {
            0x30, 0xed, 0xb6, 0x77, 0xca, 0x74, 0xe0, 0x08, 0x9d, 0xd0,
            0xe7, 0xb8, 0x85, 0x43, 0x56, 0xbb, 0x14, 0x48, 0xe3, 0x7c,
            0xdb, 0xef, 0xe7, 0xf3, 0xa8, 0x4f, 0x4f, 0x5f, 0xb3, 0xfd, 0
        };

        ExPop::CryptoRC4 rc4;
        rc4.setKey(cstest1, 15);
        rc4.encrypt(cstest2, sizeof(cstest2) - 1);
        EXPOP_TEST_VALUE(std::string((char*)cstest2), std::string("This is a test of CipherSaber."));
    }

    {
        ExPop::CryptoRC4 rc4;
        rc4.setKey("dickbutt");

        std::string base = "herpy derpy derp";
        std::string encrypted = stringBase64EncodeString(rc4.encrypt(base));

        rc4 = CryptoRC4();
        rc4.setKey("dickbutt");
        std::string decrypted = rc4.encrypt(stringBase64DecodeString(encrypted));

        EXPOP_TEST_VALUE(decrypted, base);
        EXPOP_TEST_VALUE(encrypted, "ZlX9S9jQTFeMnmaTshss4Q==");
    }
}

struct ThreadTestData
{
    ExPop::Threads::ThreadId id;
    size_t *passCounter;
    size_t *failCounter;
    bool goAhead;
};

void sleepWrapper(uint32_t time)
{
  #if _WIN32
    Sleep(time / 1000);
  #else
    usleep(time);
  #endif
}

void doThreadTests_thread(void *data)
{
    ThreadTestData *testData = (ThreadTestData*)data;

    ExPop::Threads::ThreadId thisId = ExPop::Threads::getMyId();
    size_t &passCounter = *testData->passCounter;
    size_t &failCounter = *testData->failCounter;

    while(!testData->goAhead) {
        sleepWrapper(10);
    }

  #if _WIN32
    std::cout << "[Thread] Current thread ID: " << GetCurrentThreadId() << std::endl;
  #endif

    cout << "[Thread] This ID:  " << thisId << std::endl;
    cout << "[Thread] Other ID: " << testData->id << std::endl;

    EXPOP_TEST_VALUE(thisId != testData->id, true);
}

inline void doThreadTests(size_t &passCounter, size_t &failCounter)
{
    ExPop::Threads::ThreadId thisId = ExPop::Threads::getMyId();

    ThreadTestData testData;
    testData.id = thisId;
    testData.passCounter = &passCounter;
    testData.failCounter = &failCounter;
    testData.goAhead = false;

    ExPop::Threads::Thread t(doThreadTests_thread, &testData);
    sleepWrapper(1000);

    #if _WIN32
    std::cout << "[Main  ] Current thread ID: " << GetCurrentThreadId() << std::endl;
    #endif

    cout << "[Main  ] This ID:  " << thisId << std::endl;
    cout << "[Main  ] Other ID: " << t.getId() << std::endl;
    EXPOP_TEST_VALUE(t.getId() != thisId, true);

    testData.goAhead = true;
    t.join();
}

inline void doCompressTests(size_t &passCounter, size_t &failCounter)
{
    std::string fileData = FileSystem::loadFileString("README.org");
    unsigned int len = 0;
    char *compressedData = compressRLE(&fileData[0], fileData.size(), &len, 1);

    std::cout << "Original size:   " << fileData.size() << std::endl;
    std::cout << "Compressed size: " << len << std::endl;
    EXPOP_TEST_VALUE(len < fileData.size(), true);

    unsigned int len2 = 0;
    char *uncompressedData = decompressRLE(compressedData, len, &len2, 1);
    EXPOP_TEST_VALUE(len2, fileData.size());

    EXPOP_TEST_VALUE(strncmp(&fileData[0], uncompressedData, ExPop::min(fileData.size(), (size_t)len2)), 0);

    delete[] uncompressedData;
    delete[] compressedData;
}

inline void doPreprocessorTests(size_t &passCounter, size_t &failCounter)
{
    PreprocessorState state;
    state.definedSymbols["DICK"] = "1";
    state.definedSymbols["BUTT"] = "0";

    EXPOP_TEST_VALUE(
        preprocess(
            "faketest.txt",
            "#if DICK\ndick\n#else\nbutt\n#endif\n#if BUTT\nbutt\n#else\ndick\n#endif\n",
            state),
        "dick\ndick\n");
    EXPOP_TEST_VALUE(state.hadError, false);

    EXPOP_TEST_VALUE(
        preprocess(
            "faketest.txt",
            "#endif",
            state), "");

    EXPOP_TEST_VALUE(state.hadError, true);
}

// inline void doArchiveTests(size_t &passCounter, size_t &failCounter)
// {
//     {
//         FileSystem::Archive testArchive("lilylibtest.poop", true);
//         std::string fileData = FileSystem::loadFileString("README.org");
//         EXPOP_TEST_VALUE(testArchive.getFailed(), false);
//         bool addFileSucceed = testArchive.addFile("Stuff_Only_For_Archive_Test", fileData.c_str(), fileData.size());
//         EXPOP_TEST_VALUE(addFileSucceed, true);
//         EXPOP_TEST_VALUE(testArchive.getFailed(), false);
//         bool addFileFail = testArchive.addFile("Stuff_Only_For_Archive_Test", fileData.c_str(), fileData.size());
//         EXPOP_TEST_VALUE(addFileFail, false);
//     }
//     EXPOP_TEST_VALUE(FileSystem::fileExists("lilylibtest.poop"), true);
//     EXPOP_TEST_VALUE(FileSystem::fileExists("Stuff_Only_For_Archive_Test"), false);
//     FileSystem::addArchiveForSearch("lilylibtest.poop");
//     EXPOP_TEST_VALUE(FileSystem::fileExists("Stuff_Only_For_Archive_Test"), true);
//     FileSystem::removeArchiveForSearch("lilylibtest.poop");
//     EXPOP_TEST_VALUE(FileSystem::fileExists("Stuff_Only_For_Archive_Test"), false);
//     FileSystem::deleteFile("lilylibtest.poop");
//     EXPOP_TEST_VALUE(FileSystem::fileExists("lilylibtest.poop"), false);
// }

inline void doParserTests(size_t &passCounter, size_t &failCounter)
{
    ParserNode *node = new ParserNode();
    node->setStringValue("dick", "butt");
    node->setBinaryValue("foo", "bar", 3);

    ParserNode *child = new ParserNode();
    child->setName("thingy");
    child->setStringValue("Whatever", "Arghblargh");

    node->addChildToEnd(child);

    node->addChildToEnd(parseXmlString("<junk><blagh>HERPDERP</blagh>Whatever</junk>"));

    {
        std::string badXmlError;
        ParserNode *badXml = parseXmlString("<bad_xml<", &badXmlError);
        EXPOP_TEST_VALUE(!badXmlError.size(), 0);
        delete badXml;
    }

    {
        std::string badJsonError;
        ParserNode *badJson = parseJsonString("This is obviously not valid JSON", &badJsonError);
        EXPOP_TEST_VALUE(!badJsonError.size(), 0);
        delete badJson;
    }

    cout << *node << endl;
    node->outputXml(cout, 0);
    cout << endl;
    node->outputJson(cout, 0);
    cout << endl;

    {
        ostringstream ostr;
        ostr << *node;
        ParserNode *readBack = parseString(ostr.str());
        EXPOP_TEST_VALUE(readBack->getChild(0)->getStringValueDirty("Whatever"), "Arghblargh");
        delete readBack;
    }

    {
        ostringstream ostr;
        node->outputXml(ostr, 0);
        ParserNode *readBack = parseXmlString(ostr.str());
        EXPOP_TEST_VALUE(readBack->getChild(0)->getChild(0)->getStringValueDirty("Whatever"), "Arghblargh");
        delete readBack;
    }

    {
        ostringstream ostr;
        node->outputJson(ostr, 0);
        ParserNode *readBack = parseJsonString(ostr.str());
        EXPOP_TEST_VALUE(readBack->getChild(0)->getStringValueDirty("Whatever"), "Arghblargh");
        delete readBack;
    }

    delete node;
}

inline void doFilesystemTests(const char *argv0, size_t &passCounter, size_t &failCounter)
{
    EXPOP_TEST_VALUE(system(("\"" + FileSystem::getExecutablePath(argv0) + "\" --quit").c_str()), 0);

    EXPOP_TEST_VALUE(
        FileSystem::loadFileString("tests/normal_file.txt"),
        "This file is outside a zip.\n");

    FileSystem::mountZipFile("tests/zip_test.zip");

    EXPOP_TEST_VALUE(
        FileSystem::fileExists("tests/zip_test.txt"), true);

    EXPOP_TEST_VALUE(
        FileSystem::getFileSize("tests/zip_test.txt"), 27);

    EXPOP_TEST_VALUE(
        FileSystem::loadFileString("tests/zip_test.txt"),
        "This file is inside a zip.\n");

    FileSystem::unmountAll();

    EXPOP_TEST_VALUE(
        FileSystem::fileExists("tests/zip_test.txt"), false);

    EXPOP_TEST_VALUE(
        FileSystem::getFileSize("tests/zip_test.txt"), -1);

    EXPOP_TEST_VALUE(
        FileSystem::loadFileString("tests/zip_test.txt"),
        "");
}

// ----------------------------------------------------------------------

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

// ----------------------------------------------------------------------
// Image loader test
// ----------------------------------------------------------------------

std::string getIndent(int level)
{
    std::string st;
    for(int i = 0; i < level; i++) {
        st = st + "  ";
    }
    return st;
}

void runImgTest(
    std::istream &inStream,
    bool noMoreRecursion = false,
    int indentLevel = 0)
{
    std::string inData;
    while(inStream.good()) {
        char c;
        inStream.read(&c, 1);
        inData.append(1, c);
    }

    // ----------------------------------------------------------------------
    // TGA loader

    std::cout << getIndent(indentLevel) << "Tga test..." << std::endl;
    PixelImage<uint8_t> *img = pixelImageLoadTGA(&inData[0], inData.size());
    if(img) {
        std::cout << getIndent(indentLevel + 1)
                  << "Image loaded: "
                  << img->getWidth()
                  << "x"
                  << img->getHeight()
                  << ", "
                  << img->getChannelCount()
                  << std::endl;
    } else {
        std::cout << getIndent(indentLevel + 1)
                  << "Image failed to load."
                  << std::endl;
    }

    // ----------------------------------------------------------------------
    // PNG loader (STB)

    // TODO

    // ----------------------------------------------------------------------
    // PNG loader (libpng)

    // TODO

    // ----------------------------------------------------------------------
    // JPEG loader (STB)

    // TODO

    // ----------------------------------------------------------------------
    // JPEG loader (libjpeg)

    // TODO

    // ----------------------------------------------------------------------
    // ZIP loader

    std::cout << getIndent(indentLevel) << "Zip test..." << std::endl;
    std::shared_ptr<istringstream> istr = std::shared_ptr<istringstream>(new istringstream(inData));
    std::shared_ptr<ZipFile> zf = std::shared_ptr<ZipFile>(new ZipFile(istr));
    std::vector<std::string> fileList;
    zf->fillFileList(fileList);
    std::cout << getIndent(indentLevel + 1) << "Files..." << std::endl;
    for(size_t i = 0; i < fileList.size(); i++) {
        // Recurse into all files, except ZIPs.
        std::cout << getIndent(indentLevel + 2) << replaceNonDisplayableASCII(fileList[i], '_') << std::endl;
        std::shared_ptr<istream> fileInZip = zf->openFile(fileList[i]);
        if(fileInZip) {
            if(!noMoreRecursion) {
                runImgTest(*fileInZip, true, indentLevel + 3);
            }
        } else {
            std::cout << getIndent(indentLevel + 3)
                      << "Can't open file in zip."
                      << std::endl;
        }
    }

    // ----------------------------------------------------------------------
    // ZIP mounter

    if(!noMoreRecursion) {

        std::cout << getIndent(indentLevel) << "Zip mount test..." << std::endl;

        FileSystem::mountZipFile(zf, "/fakemountpoint");

        for(size_t i = 0; i < fileList.size(); i++) {
            // Recurse into all files, except ZIPs.
            std::cout << getIndent(indentLevel + 1) << replaceNonDisplayableASCII(fileList[i], '_') << std::endl;

            std::string fileData = FileSystem::loadFileString("/fakemountpoint/" + fileList[i]);
            std::istringstream istr(fileData);
            runImgTest(istr, true, indentLevel + 2);
        }

        FileSystem::unmountAll();
    }

    // ----------------------------------------------------------------------
    // XML parser

    // TODO

    // ----------------------------------------------------------------------
    // Custom parser

    // TODO

    // ----------------------------------------------------------------------
    // JSON parser

    // TODO

    // ----------------------------------------------------------------------
    // Base64 parser

    // TODO

    // ----------------------------------------------------------------------
    // UTF-8 decoder (and re-encoding)

    // TODO

    // ----------------------------------------------------------------------
    // RLE decoder

    // TODO

    // ----------------------------------------------------------------------
    // Preprocessor

    // TODO

    delete img;
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

    bool ranSpecificTest = false;

    for(size_t i = 0; i < params.size(); i++) {

        if(params[i].name == "quit") {
            return 0;
        } else if(params[i].name == "help") {
            showHelp(argv[0], false);
            return 0;
        } else if(params[i].name == "imgloader") {
            runImgTest(cin);
            ranSpecificTest = true;
            return 0;
        } else {
            cerr << "Unrecognized parameter: " << params[i].name << endl;
            return 1;
        }
    }

    if(ranSpecificTest) {
        return 0;
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

    showSectionHeader("RC4");
    doRC4Tests(passCounter, failCounter);

    showSectionHeader("Thread");
    doThreadTests(passCounter, failCounter);

    showSectionHeader("Compression");
    doCompressTests(passCounter, failCounter);

    showSectionHeader("Preprocessor");
    doPreprocessorTests(passCounter, failCounter);

    // showSectionHeader("Archive");
    // doArchiveTests(passCounter, failCounter);

    showSectionHeader("Parser");
    doParserTests(passCounter, failCounter);

    showSectionHeader("Filesystem");
    doFilesystemTests(argv[0], passCounter, failCounter);

    showSectionHeader("Results");
    std::cout << "Passed: " << passCounter << std::endl;
    std::cout << "Failed: " << failCounter << std::endl;
    cout << std::endl;

    if(failCounter) {
        return 1;
    }

    return 0;
}


