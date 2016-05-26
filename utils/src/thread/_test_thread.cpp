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

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include <time.h>

#include "thread.h"
using namespace ExPop;
using namespace Threads;

Mutex mainMut;
Mutex outputMut;

void bar(void *data) {

    const char *blargh = (char*)data;

    cout << "HERP" << endl;

    outputMut.lock();
    cout << "Thread started: " << getMyId() << " with string " << blargh << endl;
    outputMut.unlock();

    cout << "DERP" << endl;

    cout << "FOO" << endl;

    for(int i = 0; i < 10; i++) {
        mainMut.lock();
        // mainVec->push_back(blargh);
        mainMut.unlock();
    }

    cout << "BAR" << endl;

/*#if _WIN32
    Sleep(50);
    #endif*/

}

int main(int argc, char *argv[]) {

    vector<string> mainVec;

    cout << "Main thread: " << Threads::getMyId() << endl;

    Thread gt(bar, (void*)"DER");
    Thread foo = Thread(bar, (void*)"Moo");
    Thread guh = Thread(bar, (void*)"Blagh");

    outputMut.lock();

    //cout << "Thread IDs from the main thread: " << foo.getId() << " " << guh.getId() << endl;

    outputMut.unlock();

    cout << "JOINING EVERYTHING" << endl;

    foo.join();
    guh.join();
    gt.join();

    cout << "OUTPUT" << endl;

    for(unsigned int i = 0; i < mainVec.size(); i++) {
        cout << mainVec[i] << endl;
    }

    cout << "DONE" << endl;

    return 0;
}
