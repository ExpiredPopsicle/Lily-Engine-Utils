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
#include <vector>
#include <cstring>
using namespace std;

#include <lilyengine/utils.h>
#include <lilyengine/malstring.h>
using namespace ExPop;
using namespace ExPop::Gfx;

#include "usagetext.h"

void showHelp(const char *argv0)
{
    cout << stringReplace<char>("$0", argv0, std::string(usageText, usageText_len)) << endl;
}

int main(int argc, char *argv[])
{
    // Parse params.
    std::vector<std::string> paramNames = { };
    std::vector<ParsedParameter> params;
    parseCommandLine(argc, argv, paramNames, params);

    std::string filename;
    std::string variableName;

    for(size_t i = 0; i < params.size(); i++) {
        if(params[i].name == "help") {
            showHelp(argv[0]);
            return 0;
        } else {
            if(!filename.size()) {
                filename = params[i].value;
            } else if(!variableName.size()) {
                variableName = params[i].value;
            } else {
                cerr << "Unknown parameter: " << params[i].name << endl;
            }
        }
    }

    if(argc < 3) {
        showHelp(argv[0]);
        return 1;
    }

    int len = 0;
    char *buf = FileSystem::loadFile(filename, &len);

    if(!buf) {
        cerr << "Error loading file: " << filename << endl;
        return 1;
    }

    Image *img = loadTGA(buf, len);
    if(!img) {
        cerr << "Error loading TGA data from " << filename << " possibly a malformed or unsupported TGA." << endl;
        return 1;
    }

    unsigned int width  = img->getWidth();
    unsigned int height = img->getHeight();

    if((width * height) % 8) {
        cerr << "Image must have a multiple of eight for the number of pixels." << endl;
        return 1;
    }

    // Allocate the buffer.
    unsigned int outBufLen = (width * height) / 8;
    char *outBuf = new char[outBufLen];
    memset(outBuf, 0, outBufLen);

    // Generate the 1-bit-per-pixel buffer for the image here.
    unsigned int byteIndex = 0;
    unsigned int bitIndex = 0;
    for(unsigned int y = 0; y < height; y++) {
        for(unsigned int x = 0; x < width; x++) {

            Pixel *px = img->getPixel(x, y);

            unsigned int total =
                (unsigned int)px->rgba.r +
                (unsigned int)px->rgba.g +
                (unsigned int)px->rgba.b;

            bool white = (total > 384);

            if(white) {
                outBuf[byteIndex] |= (1 << (7-bitIndex));
            }

            bitIndex++;
            if(bitIndex >= 8) {
                bitIndex = 0;
                byteIndex++;
            }

        }
    }

    cout << "namespace ExPop {" << endl;
    cout << "    namespace Gfx {" << endl;
    cout << "        unsigned int " << variableName << "_width  = " << img->getWidth() << ";" << endl;
    cout << "        unsigned int " << variableName << "_height = " << img->getHeight() << ";" << endl;
    cout << "        unsigned int " << variableName << "_length = " << outBufLen << ";" << endl;
    cout << "        char " << variableName << "_data[]         = {" << endl;

    char hexLookup[] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F' };

    unsigned int columns = 20;

    for(unsigned int i = 0; i < outBufLen; i++) {

        // Handle indents, newlines, and blah.
        if(!(i % columns)) {
            if(i) {
                cout << endl;
            }
            cout << "            ";
        }

        cout << "0x" <<
            hexLookup[(outBuf[i] & 0xF0) >> 4] <<
            hexLookup[outBuf[i] & 0x0F] << ",";

        if((i % columns) != columns - 1 && i != (outBufLen - 1)) {
            cout << " ";
        }
    }

    cout << endl;

    cout << "        };" << endl;
    cout << "    }" << endl;
    cout << "}" << endl;

    delete img;
    delete[] outBuf;
    delete[] buf;


    return 0;
}
