#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
using namespace std;

#include "derpobject.h"
#include "derpvm.h"
using namespace ExPop;

DerpObject::Ref testExternalFunc(
    DerpObject::ExternalFuncData &data) {

    cout << "Calling external test function!" << endl;

    DerpObject::Ref ret = data.vm->makeObject();
    ret->setInt(3);

    // if(data.parameters.size() &&
    //    data.parameters[0]->getType() == DERPTYPE_INT &&
    //    data.parameters[0]->getInt() == 2) {

    //     data.vm->addError("HURF BLURF");
    //     return NULL;
    // }

    cout << "----------------------------------------------------------------------" << endl;

    return ret;
}

class CustomTest : public DerpObject::CustomData {
public:
    std::string testString;
};

DerpObject::Ref customTestFunc(DerpObject::ExternalFuncData &data) {

    if(data.parameters.size() == 1) {
        if(data.parameters[0]->getType() == DERPTYPE_CUSTOMDATA) {
            CustomTest *ct = dynamic_cast<CustomTest*>(data.parameters[0]->getCustomData());
            if(ct) {
                cout << "HOLY BALLS CUSTOM DATA WORKS: " << ct->testString << endl;
            }
        }
    }

    DerpObject::Ref ret = data.vm->makeObject();
    ret->setInt(0);
    return ret;
}

int main(int argc, char *argv[]) {

    DerpErrorState errorState;

    DerpObject::Ref refToHold;

    DerpVM *vm = new DerpVM();
    {
        DerpContext *ctx = vm->getGlobalContext();
        DerpObject::Ref testFuncOb(new DerpObject(vm));
        testFuncOb->setExternalFunction(testExternalFunc);
        ctx->setVariable("testExternalFunc", testFuncOb);
        testFuncOb->setConst(true);
        ctx->setVariableProtected("testExternalFunc", true);


        DerpObject::Ref key = vm->makeObject();
        key->setString("blurp");

        DerpObject::Ref val = vm->makeObject();
        val->setString("balgh");

        DerpObject::Ref testTable = vm->makeObject();
        testTable->setTable();
        // testTable->setInTable(key, val);

        DerpObject::Ref testTable2 = vm->makeObject();
        testTable2->setTable();
        testTable2->setInTable(key, testTable);

        testTable->setInTable(key, testTable2);

        // cout << testTable->getInTable(key)->debugString() << endl;

        // testTable->setConst(true);
        ctx->setVariable("testTable", testTable);

        refToHold = testTable;
    }

    {
        DerpContext *ctx = vm->getGlobalContext();
        CustomTest *ct = new CustomTest;
        ct->testString = "DICKSDICKSDICKS";
        DerpObject::Ref customDataTest = vm->makeObject();
        customDataTest->setCustomData(ct);
        ctx->setVariable("customTest", customDataTest);

        customDataTest = vm->makeObject();
        customDataTest->setCustomData(ct);
        ctx->setVariable("customTest2", customDataTest);

        DerpObject::Ref customTestFuncOb = vm->makeObject();
        customTestFuncOb->setExternalFunction(customTestFunc);
        ctx->setVariable("customTestFunc", customTestFuncOb);

        cout << "Number of refs for customTest: " << vm->getNumCustomDataRefs(ct) << endl;
    }




    DerpObject::Ref ob;

    // Load a test file in the ugliest way possible.
    FILE *in = fopen("test.derp", "rb");
    ostringstream fstr;
    int inchar = fgetc(in);
    while(inchar != EOF) {
        fstr << (char)inchar;
        inchar = fgetc(in);
    }
    fclose(in);

    // cout << fstr.str() << endl;

    ob = vm->evalString(
        fstr.str(), errorState, "test.derp");

    if(!ob) {
        cout << "Errors..." << endl;
        cout << errorState.getAllErrorText() << endl;
        assert(0);
    }

    cout << "First eval complete." << endl;

    for(unsigned int i = 0; i < 100; i++) {

        DerpObject::Ref funcToTest;

        // Test different ways to get a variable.
        if(i % 2) {
            funcToTest = vm->evalString("blaghblagh;", errorState, "internal");
        } else {
            DerpContext *ctx = vm->getGlobalContext();
            funcToTest = ctx->getVariable("blaghblagh");
        }

        if(funcToTest) {

            // Test calling a function in the VM.
            DerpObject::Ref result = funcToTest->evalFunction(NULL, NULL, NULL, errorState);
            cout << "Result of test: " << result->debugString() << endl;
        }
    }


    if(ob) {
        cout << "Final result: " << ob->debugString() << endl;
    } else {
        cout << "evalString failed." << endl;
    }

    vm->garbageCollect();
    cout << "Final object count: " << vm->getNumObjects() << endl;


    cout << "Nuking VM" << endl;

    errorState.reset();

    // refToHold->setInt(0);

    delete vm;

    cout << "DONE" << endl;

    return 0;
}



