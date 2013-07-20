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

    // Otherwise, load, compile, and run stuff.
    string source = FileSystem::loadFileString(argv[1]);

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


