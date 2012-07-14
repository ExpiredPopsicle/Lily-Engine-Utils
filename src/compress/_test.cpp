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

#include <iostream>
#include <iomanip>
using namespace std;

#include <string.h>

#include "../filesystem/filesystem.h"
#include "compress.h"

void dumpMem(const char *mem, unsigned int length) {

	while(length) {

		for(int i = 0; i < 16; i++) {
			cout << setw(2) << hex << (int)(unsigned char)(*mem) << " ";
			mem++;
			length--;
			if(!length) break;
		}
		cout << endl;
	}
}

int main(int argc, char *argv[]) {

	const char *testStr = "Wharrgarbl! ...........fffffffffffffffffff";
	//const char *testStr = "asdfasdfasdfasdf";
	//const char *testStr = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	int testLength = strlen(testStr) + 1;

	if(argc > 1) {
		testStr = FileSystem::loadFile(argv[1], &testLength);
	}

	if(!testStr) {
		cout << "Can't load that." << endl;
		return 1;
	}

	unsigned int doneLength = 0;
	unsigned int undoneLength = 0;

	char *compressed = compressRLE(testStr, testLength, &doneLength, 4);
	char *uncompressed = decompressRLE(compressed, doneLength, &undoneLength, 4);

	cout << "Non-compressed size: " << testLength << endl;
	cout << "Compressed size:     " << doneLength << endl;
	cout << "Uncompressed size:   " << undoneLength << endl;

	if(argc < 2) {

		cout << "original:     " << testStr << endl;
		cout << "decompressed: " << (uncompressed ? uncompressed : "") << endl;

		cout << endl << "Uncompressed dump:" << endl;
		dumpMem(testStr, testLength);

		cout << endl << "Compressed dump:" << endl;
		dumpMem(compressed, doneLength);

		if(uncompressed) {
			cout << endl << "Decompressed dump:" << endl;
			dumpMem(uncompressed, undoneLength);
		}

	} else {

		if(uncompressed) {

			bool verified = true;
			for(int i = 0; i < testLength; i++) {
				if(uncompressed[i] != testStr[i]) {
					verified = false;
					break;
				}
			}

			if(verified) {
				cout << "Verified data" << endl;
			} else {
				cout << "Something went horribly wrong!" << endl;
			}
		}
	}

	delete[] compressed;
	delete[] uncompressed;

	if(argc > 1) {
		delete[] testStr;
	}

	return 0;
}


