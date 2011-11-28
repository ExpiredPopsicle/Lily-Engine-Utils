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

// This is just a really dirty test of most of the FileSystem
// functionality. If you run it, it'll leave crap all over the place,
// but it shows how to use some of the features.

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "filesystem.h"
#include "archive.h"
using namespace ExPop;
using namespace FileSystem;

int main(int argc, char *argv[]) {

    // Show a listing of the current directory.

	vector<string> fileList;

	getAllFiles(".", fileList);

	for(int i = 0; i < (int)fileList.size(); i++) {
		cout << setw(20) << right << fileList[i] << " ";
		if(isDir(fileList[i])) {
			cout << "<DIR>" << endl;
		} else {
			cout << setw(20) << left << getFileSize(fileList[i]) << endl;
		}
	}

    // Load some files and try saving them again.

	int dataLen;
	char *data = loadFile("TODO", &dataLen);
	if(data) {
		for(int i = 0; i < dataLen; i++) {
			char c = data[i];
			cout << setfill('0') << setw(2) << right <<setbase(16) << (int)c << " " << c << endl;
		}
  	} else {
		cout << "Couldn't open \"TODO\"" << endl;
	}

	if(saveFile("TODO_copy", data, dataLen)) {
		cout << "Save fail." << endl;
	}

    // Delete and copy files.

	deleteFile("TODO_copy");

	copyFile("README", "test1");
	copyFile("TODO", "test1");

    // Create an archive file with the stuff we loaded from the "TODO"
    // file and try loading from it again.

	Archive *testArc = new Archive("testArc.derp", true);
	testArc->addFile("herp", data, dataLen);
	testArc->addFile("derp", data, dataLen);
	testArc->addFile("blargh/foobar", data, dataLen);
	testArc->addFile("warg", data, dataLen);
	testArc->addFile("foooooooooo/bar", data, dataLen);
	testArc->addFile("ASDF ASDF ASDF ASDF", data, dataLen);
	testArc->addFile("*.*", data, dataLen);
	delete testArc;

	testArc = new Archive("testArc.derp");

	int dumpLen;
	char *dump = testArc->loadFile("warg", &dumpLen);
	delete testArc;

    // Save the loaded file out again so we can make sure the archives
    // are actually preserving it with a simple diff.

	saveFile("warg.txt", dump, dumpLen);

	delete data;

	return 0;
}


