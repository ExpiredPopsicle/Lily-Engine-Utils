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

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstring>
#include <cstdlib>
using namespace std;

#include "../string/malstring.h"
#include "lilyparser.h"
using namespace ExPop;

int main(int argc, char *argv[]) {

    const char *testBuf = "# Herp derp this is a comment.\ncheese; foo= \"bar\"; wark { asdf=\"mo\\\"\\nof\"; blah {\n} derp { } foo { a = \"b\"; } }";

    std::string errStr;

    ParserNode *node = parseBuffer(testBuf, strlen(testBuf), &errStr);

    if(node) {

        cout << "--------" << endl << *node << endl;

        for(int i = 0; i < 100; i++) {
            ostringstream str;
            str << *node;

            delete node;

            //node = parseBuffer(str.str().c_str(), str.str().size());
            node = parseString(str.str());
        }

        cout << "--------" << endl << *node << endl;

        delete node;

    } else {

        cout << "Parser error: " << errStr << endl;

    }

    return 0;
}

