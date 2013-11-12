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

// This file can serve as an example of the DerpScript API. It should
// be pretty simple.

#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
using namespace std;

#include <lilyengine/derpvm.h>
#include <lilyengine/derpobject.h>
#include <lilyengine/utils.h>
#include <lilyengine/filesystem.h>
using namespace ExPop;


DerpObject::Ref inFunc(DerpObject::ExternalFuncData &data) {
    ostringstream outStr;
    int c = getchar();
    while(c != EOF) {
        outStr << (char)c;
        c = getchar();
    }
    DerpObject::Ref r = data.vm->makeObject();
    r->setString(outStr.str());
    return r;
}

DerpObject::Ref outFunc(DerpObject::ExternalFuncData &data) {

    for(unsigned int i = 0; i < data.parameters.size(); i++) {
        if(data.parameters[i]->getType() == DERPTYPE_STRING) {
            cout << data.parameters[i]->getString();
        } else {
            cout << data.parameters[i]->debugString();
        }
    }

    DerpObject::Ref r = data.vm->makeObject();
    r->setInt(0);
    return r;
}

void runREPL(DerpVM &vm) {

    DerpContext *globalContext = vm.getGlobalContext();

    while(cin.good()) {

        string line;
        cout << "> ";
        getline(cin, line);

        if(cin.good()) {

            DerpErrorState errorState;

            // Compile and check for errors.
            DerpObject::Ref ob = vm.compileString(line, errorState, "stdin");
            if(errorState.getNumErrors()) {
                cerr << errorState.getAllErrorText() << endl;
                continue;
            }

            // Run it!
            DerpObject::Ref result = ob->evalFunction(
                globalContext,
                NULL, NULL,
                errorState,
                true);

            if(errorState.getNumErrors()) {
                cerr << errorState.getAllErrorText() << endl;
                continue;
            }

            cout << result->debugString() << endl;

        }

    }

    cout << endl;
}

void initVM(DerpVM &vm) {

    // Make a couple of callback functions, and set them as global
    // variables.

    DerpObject::Ref inFuncOb = vm.makeObject();
    inFuncOb->setExternalFunction(inFunc);
    vm.getGlobalContext()->setVariable("in", inFuncOb);

    DerpObject::Ref outFuncOb = vm.makeObject();
    outFuncOb->setExternalFunction(outFunc);
    vm.getGlobalContext()->setVariable("out", outFuncOb);

    // // If we're running untrusted code, set an execution limit so it
    // // can't just loop forever.
    // vm.setExecNodeLimit(1000000);
}

int main(int argc, char *argv[]) {

    DerpVM vm;
    initVM(vm);

    // No parameters? Just run the REPL.
    if(argc < 2) {
        runREPL(vm);
        return 0;
    }

    // TODO: Save command line parameters after the script name so
    // that we can pass them to the script somehow.

    // Otherwise, load, compile, and run stuff.
    string source = FileSystem::loadFileString(argv[1]);

    // Special-case to get rid of the hashbang ("#!"). Because we
    // don't actually recognize '#' as a comment.
    if(strStartsWith("#!", source)) {
        // Just replace the #! with our comment marker.
        source[0] = '/';
        source[1] = '/';
    }

    if(!source.size()) {
        cerr << "Cannot load " << argv[1] << endl;
        return 1;
    }

    DerpErrorState errorState;

    DerpObject::Ref ob = vm.compileString(
        source.c_str(), errorState, argv[1]);

    if(errorState.getNumErrors()) {
        cerr << errorState.getAllErrorText() << endl;
        return 1;
    }

    DerpObject::Ref result = ob->evalFunction(
        vm.getGlobalContext(),
        NULL, NULL,
        errorState,
        true);

    if(errorState.getNumErrors()) {
        cerr << errorState.getAllErrorText() << endl;
        return 1;
    }

    return 0;
}


