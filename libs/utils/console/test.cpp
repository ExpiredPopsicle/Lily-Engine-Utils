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

// FIXME: This doesn't compile since I removed Boost from the project.

#include "../hacks/winhacks.h"
#include "../thread/thread.h"

#include <boost/bind.hpp>

#include <iostream>

using namespace std;

#include "console.h"

using namespace ExPop::Console;

bool quit = false;

void runAroundScreaming(int a, int b) {

	while(!quit) {
        Threads::ThreadId id = Threads::getMyId();
		out(a) << "Thing foo bar: " << a << " " << b << " id: " << id << endl;
	}
}

int main(int argc, char *argv[]) {

    cout << "Starting main " << Threads::getMyId() << endl;

	outSetAttribs("1", "31;1");
	outSetAttribs("3", "32;1");

    Threads::Thread t1(boost::bind(&runAroundScreaming, 1, 2));
    Threads::Thread t2(boost::bind(&runAroundScreaming, 3, 4));

	int i = 10;
	while(i) {
		usleep(10000);
		out(4) << "Derp derp " << i << endl;
		i--;
	}

	quit = true;

	t1.join();
	t2.join();

	return 0;
}

