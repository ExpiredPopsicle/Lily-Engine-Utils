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

#include "derpvm_builtinfunctions.h"

#include <iostream>
#include <cassert>
using namespace std;

#include "derpexecnode.h"
#include "derpobject.h"
#include "derpvm.h"
#include "derpparser.h"
#include "derpconfig.h"

namespace ExPop {

    // ----------------------------------------------------------------------
    //  Built-in callbacks
    // ----------------------------------------------------------------------

    static DerpObject::Ref toFloat(DerpObject::ExternalFuncData &data) {

        if(data.parameters.size() != 1) {
            data.errorState->addError(
                "Incorrect number of parameters for float typecast function.");
            return NULL;
        }

        DerpObject::Ref ret = data.vm->makeObject();
        DerpObject::Ref ob = data.parameters[0];

        switch(ob->getType()) {

            case DERPTYPE_INT:
                ret->setFloat((float)ob->getInt());
                break;

            case DERPTYPE_FLOAT:
                ret->setFloat(ob->getFloat());
                break;

            case DERPTYPE_STRING: {
                istringstream inStr(ob->getString());
                float f;
                inStr >> f;
                ret->setFloat(f);
            } break;

            default:
                data.errorState->addError(
                    "Object type cannot be converted to float: " +
                    derpObjectTypeToString(ob->getType()));
                return NULL;
        }

        return ret;
    }

    static DerpObject::Ref toInteger(DerpObject::ExternalFuncData &data) {

        if(data.parameters.size() != 1) {
            data.errorState->addError(
                "Incorrect number of parameters for integer typecast function.");
            return NULL;
        }

        DerpObject::Ref ret = data.vm->makeObject();
        DerpObject::Ref ob = data.parameters[0];

        switch(ob->getType()) {

            case DERPTYPE_INT:
                ret->setInt(ob->getInt());
                break;

            case DERPTYPE_FLOAT:
                ret->setInt(ob->getFloat());
                break;

            case DERPTYPE_STRING: {
                istringstream inStr(ob->getString());
                int i;
                inStr >> i;
                ret->setInt(i);
            } break;

            default:
                data.errorState->addError(
                    "Object type cannot be converted to integer: " +
                    derpObjectTypeToString(ob->getType()));
                return NULL;
        }

        return ret;
    }


    static DerpObject::Ref toString(DerpObject::ExternalFuncData &data) {

        if(data.parameters.size() != 1) {
            data.errorState->addError(
                "Incorrect number of parameters for string typecast function.");
            return NULL;
        }

        DerpObject::Ref ret = data.vm->makeObject();
        DerpObject::Ref ob = data.parameters[0];
        ostringstream outStr;

        switch(ob->getType()) {

            case DERPTYPE_INT:
                outStr << ob->getInt();
                break;

            case DERPTYPE_FLOAT:
                outStr << ob->getFloat();
                break;

            case DERPTYPE_STRING:
                outStr << ob->getString();
                break;

            default:
                data.errorState->addError(
                    "Object type cannot be converted to string: " +
                    derpObjectTypeToString(ob->getType()));
                return NULL;
        }

        ret->setString(outStr.str());
        return ret;
    }

    static DerpObject::Ref makeTable(DerpObject::ExternalFuncData &data) {
        DerpObject::Ref ob = data.vm->makeObject();
        ob->setTable();

        for(unsigned int i = 0; i < data.parameters.size(); i++) {
            DerpObject::Ref key = data.vm->makeObject();
            key->setInt(i);
            ob->setInTable(key, data.parameters[i]);
        }

        return ob;
    }

    static DerpObject::Ref table_isSet(DerpObject::ExternalFuncData &data) {

        if(data.parameters.size() != 2) {
            data.errorState->addError(
                "Incorrect number of parameters for table_isSet.");
            return NULL;
        }

        DerpBasicType dt = data.parameters[0]->getType();
        if(dt != DERPTYPE_TABLE) {
            data.errorState->addError(
                "Non-table passed to table_isSet.");
            return NULL;
        }

        dt = data.parameters[1]->getType();
        if(!data.parameters[0]->isValidKeyType(dt)) {
            data.errorState->addError(
                "Invalid key type passed to table_isSet.");
            return NULL;
        }

        DerpObject::Ref r = data.parameters[0]->getInTable(data.parameters[1]);
        DerpObject::Ref ret = data.vm->makeObject();

        if(r) {
            ret->setInt(1);
        } else {
            ret->setInt(0);
        }

        return ret;
    }

    static DerpObject::Ref table_unSet(DerpObject::ExternalFuncData &data) {

        if(data.parameters.size() != 2) {
            data.errorState->addError(
                "Incorrect number of parameters for table_unSet.");
            return NULL;
        }

        DerpBasicType dt = data.parameters[0]->getType();
        if(dt != DERPTYPE_TABLE) {
            data.errorState->addError(
                "Non-table passed to table_unSet.");
            return NULL;
        }

        dt = data.parameters[1]->getType();
        if(!data.parameters[0]->isValidKeyType(dt)) {
            data.errorState->addError(
                "Invalid key type passed to table_unSet.");
            return NULL;
        }

        if(data.parameters[0]->getConst()) {
            data.errorState->addError(
                "Tried to modify a const table.");
            return NULL;
        }

        data.parameters[0]->clearInTable(data.parameters[1]);

        DerpObject::Ref ret = data.vm->makeObject();
        ret->setInt(0);
        return ret;
    }

    static DerpObject::Ref table_size(DerpObject::ExternalFuncData &data) {

        DerpBasicType dt = data.parameters[0]->getType();
        if(dt != DERPTYPE_TABLE) {
            data.errorState->addError(
                "Non-table passed to table_size.");
            return NULL;
        }

        DerpObject::Ref ret = data.vm->makeObject();
        ret->setInt((int)data.parameters[0]->getTableSize());
        return ret;
    }

    static DerpObject::Ref table_keys(DerpObject::ExternalFuncData &data) {

        DerpBasicType dt = data.parameters[0]->getType();
        if(dt != DERPTYPE_TABLE) {
            data.errorState->addError(
                "Non-table passed to table_keys.");
            return NULL;
        }

        DerpObject::Ref ret = data.vm->makeObject();
        ret->setTable();

        std::vector<DerpObject::Ref> keysArray = data.parameters[0]->getTableKeys();

        for(size_t i = 0; i < keysArray.size(); i++) {

            DerpObject::Ref keyOb = data.vm->makeObject();
            keyOb->setInt(i);

            ret->setInTable(keyOb, keysArray[i]);
        }

        return ret;
    }

    static void addInternalFunction(
        DerpVM *vm,
        DerpContext *context,
        const std::string &name,
        DerpObject::ExternalFunction func) {

        DerpObject::Ref funcOb;
        funcOb = vm->makeObject();
        funcOb->setExternalFunction(func);
        funcOb->setConst(true);
        context->setVariable(name, funcOb);
        context->setVariableProtected(name, true);
    }


    void derpVM_registerInternalFunctions(DerpVM *vm, DerpContext *ctx) {
        addInternalFunction(vm, ctx, "float", toFloat);
        addInternalFunction(vm, ctx, "int", toInteger);
        addInternalFunction(vm, ctx, "string", toString);
        addInternalFunction(vm, ctx, "table", makeTable);
        addInternalFunction(vm, ctx, "table_isSet", table_isSet);
        addInternalFunction(vm, ctx, "table_unSet", table_unSet);
        addInternalFunction(vm, ctx, "table_size", table_size);
        addInternalFunction(vm, ctx, "table_keys", table_keys);
    }
}




