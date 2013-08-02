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
#include <sstream>
#include <cassert>
using namespace std;

#include "derpvm.h"
#include "derpobject.h"
#include "derpexecnode.h"
#include "derpconfig.h"

namespace ExPop {

    bool DerpObjectCompare::operator()(const DerpObject::Ref &a, const DerpObject::Ref &b) const {
        return *a.getPtr() < *b.getPtr();
    }

    DerpObject::DerpObject(DerpVM *vm) {

        assert(vm);

        type = DERPTYPE_NONE;
        externalRefCount = 0;
        lastGCPass = 0;

        // Must be set to NULL for registerObject to work.
        this->vm = NULL;

        isConst = false;
        isCopyable = true;

        vm->registerObject(this);

    }

    DerpObject::~DerpObject(void) {

        if(vm) {
            vm->unregisterObject(this);
        }

        clearData();
    }

    void DerpObject::clearData(void) {

        switch(type) {
            case DERPTYPE_STRING:
                delete strVal;
                strVal = NULL;
                break;
            case DERPTYPE_INT:
            case DERPTYPE_FLOAT:
            case DERPTYPE_NONE:
                break;

            case DERPTYPE_TABLE:
                delete tableData;
                tableData = NULL;
                break;

            case DERPTYPE_CUSTOMDATA:
                vm->removeCustomDataRef(customData);
                break;

            case DERPTYPE_FUNCTION:

                if(functionData.callCounter) {

                    // Function is being called. Can't clear a tree
                    // of DerpExecNodes while it's being executed!

                    // DerpExecNode's assignment type has protections
                    // against this.
                    assert(0);

                }

                delete functionData.execNode;
                functionData.execNode = NULL;

                functionData.externalFunc = NULL;

                delete functionData.paramNames;
                functionData.paramNames = NULL;

                break;

                // TODO: User data.

            default:
                // FIXME: Unimplemented type.
                assert(0);
        }

        type = DERPTYPE_NONE;

    }

    void DerpObject::setString(const std::string &str) {
        clearData();
        strVal = new std::string();
        *(this->strVal) = str;
        type = DERPTYPE_STRING;
    }

    std::string DerpObject::getString(void) {
        assert(type == DERPTYPE_STRING);
        return *strVal;
    }

    void DerpObject::setInt(int i) {
        clearData();
        intVal = i;
        type = DERPTYPE_INT;
    }

    int DerpObject::getInt(void) {
        assert(type == DERPTYPE_INT);
        return intVal;
    }

    void DerpObject::setFloat(float f) {
        clearData();
        floatVal = f;
        type = DERPTYPE_FLOAT;
    }

    float DerpObject::getFloat(void) {
        assert(type == DERPTYPE_FLOAT);
        return floatVal;
    }

    void DerpObject::setFunction(DerpExecNode *node, std::vector<std::string> *paramNames) {
        clearData();

        functionData.execNode = node;
        functionData.callCounter = 0;
        functionData.paramNames = paramNames;
        functionData.externalFunc = NULL;
        type = DERPTYPE_FUNCTION;
    }

    void DerpObject::setExternalFunction(ExternalFunction func) {
        clearData();

        functionData.execNode = NULL;
        functionData.callCounter = 0;
        functionData.paramNames = NULL;
        functionData.externalFunc = func;
        type = DERPTYPE_FUNCTION;
    }

    void DerpObject::setTable(void) {
        clearData();
        tableData = new std::map<DerpObject::Ref, DerpObject::Ref, DerpObjectCompare>();
        type = DERPTYPE_TABLE;
    }

    void DerpObject::setInTable(DerpObject::Ref key, DerpObject::Ref value) {

        assert(type == DERPTYPE_TABLE);

        (*tableData)[key] = value;
    }

    void DerpObject::clearInTable(DerpObject::Ref key) {

        assert(type == DERPTYPE_TABLE);

        tableData->erase(key);
    }

    DerpObject::Ref DerpObject::getInTable(DerpObject::Ref key) {

        assert(type == DERPTYPE_TABLE);

        if(tableData->count(key)) {
            return (*tableData)[key];
        }

        return NULL;
    }

    DerpObject::Ref *DerpObject::getInTablePtr(DerpObject::Ref key) {

        assert(type == DERPTYPE_TABLE);

        if(tableData->count(key)) {
            return &((*tableData)[key]);
        }

        return NULL;
    }

    bool DerpObject::isValidKeyType(DerpBasicType t) {
        return
            t == DERPTYPE_STRING ||
            t == DERPTYPE_INT ||
            t == DERPTYPE_FLOAT ||
            t == DERPTYPE_CUSTOMDATA;
    }

    void DerpObject::setCustomData(DerpObject::CustomData *customData) {
        clearData();
        this->customData = customData;
        type = DERPTYPE_CUSTOMDATA;
        vm->addCustomDataRef(customData);
    }

    DerpObject::CustomData *DerpObject::getCustomData(void) {
        return customData;
    }


    void DerpObject::set(DerpObject *otherOb) {

        // Assignment operator should have protections against these things...

        // Can't modify a const.
        assert(!isConst);

        // Can't copy a unique thing.
        assert(otherOb->isCopyable);

        if(otherOb == this) return;

        clearData();

        switch(otherOb->type) {

            case DERPTYPE_STRING:
                strVal = new string();
                *(strVal) = *(otherOb->strVal);
                break;

            case DERPTYPE_INT:
                intVal = otherOb->intVal;
                break;

            case DERPTYPE_FLOAT:
                floatVal = otherOb->floatVal;
                break;

            case DERPTYPE_TABLE:
                setTable();
                *tableData = *otherOb->tableData;
                break;

            case DERPTYPE_CUSTOMDATA:
                setCustomData(otherOb->customData);
                break;

            case DERPTYPE_NONE:
                break;

            case DERPTYPE_FUNCTION:

                // Copy the entire execnode tree. This is a big copy, so
                // users should probably just use the reference copy for
                // it instead?
                functionData.execNode = otherOb->functionData.execNode ? otherOb->functionData.execNode->copyTree() : NULL;
                functionData.callCounter = 0;
                functionData.externalFunc = otherOb->functionData.externalFunc;

                if(otherOb->functionData.paramNames) {
                    functionData.paramNames = new vector<string>();

                    // TODO: Faster vector copy with iterators.
                    for(unsigned int i = 0; i < otherOb->functionData.paramNames->size(); i++) {
                        functionData.paramNames->push_back((*otherOb->functionData.paramNames)[i]);
                    }
                } else {
                    functionData.paramNames = NULL;
                }

                // TODO: Spit out a warning for function copying?
                break;

                // TODO: Add other types.

            default:
                assert(0);
        }

        type = otherOb->type;
    }

    DerpObject *DerpObject::copy(void) {

        DerpObject *ob = new DerpObject(vm);
        ob->set(this);
        return ob;
    }

    void DerpObject::markGCPass(unsigned int passNum) {

        if(lastGCPass == passNum) return;

        lastGCPass = passNum;

        switch(type) {

            case DERPTYPE_FUNCTION:
                // Recurse into exec nodes to try to find literal values.
                if(functionData.execNode) {
                    functionData.execNode->markGCPass(passNum);
                }
                break;

            case DERPTYPE_TABLE: {
                std::map<DerpObject::Ref, DerpObject::Ref, DerpObjectCompare>::iterator mapIter;
                for(mapIter = tableData->begin(); mapIter != tableData->end(); mapIter++) {
                    if((*mapIter).first.getPtr()) {
                        (*mapIter).first.getPtr()->markGCPass(passNum);
                    }
                    if((*mapIter).second.getPtr()) {
                        (*mapIter).second.getPtr()->markGCPass(passNum);
                    }
                }
            } break;

            default:
                // Nothing to do?
                break;
        }
    }

    // ----------------------------------------------------------------------
    // Debugging
    // ----------------------------------------------------------------------

    void DerpObject::dumpTable(
        std::ostream &ostr,
        std::map<const DerpObject*, bool> &stuffAlreadyDone,
        bool isRoot) const {

        assert(type == DERPTYPE_TABLE);

        if(stuffAlreadyDone[this]) return;
        stuffAlreadyDone[this] = true;

        vector<DerpObject::Ref> otherTablesWeNeed;

        ostr << (isRoot ? "rootTable" : "table") << this << " {" << endl;

        for(std::map<DerpObject::Ref, DerpObject::Ref, DerpObjectCompare>::iterator i = tableData->begin();
            i != tableData->end(); i++) {

            // Possible infinite recursion here if the spec (heh) ever
            // changes so that tables can be keys.
            ostr << "    " << (*i).first->debugString();

            ostr << " = ";

            if((*i).second->getType() == DERPTYPE_TABLE) {
                ostr << "table" << (*i).second.getPtr();
                otherTablesWeNeed.push_back((*i).second);
            } else {
                ostr << (*i).second->debugString();
            }
            ostr << endl;
        }

        ostr << "}" << endl;

        for(unsigned int i = 0; i < otherTablesWeNeed.size(); i++) {
            otherTablesWeNeed[i]->dumpTable(ostr, stuffAlreadyDone, false);
        }

    }

    std::string DerpObject::debugString(void) const {

        ostringstream outStr;

        switch(type) {

            case DERPTYPE_STRING:
                outStr << "\"" << *(strVal) << "\"";
                break;

            case DERPTYPE_INT:
                outStr << intVal;
                break;

            case DERPTYPE_FLOAT:
                outStr << floatVal;
                break;

            case DERPTYPE_NONE:
                outStr << "NULL";
                break;

            case DERPTYPE_TABLE: {
                std::map<const DerpObject *, bool> visitedStuff;
                dumpTable(outStr, visitedStuff, true);
            } break;

            case DERPTYPE_FUNCTION:
                outStr << "<function>";
                if(functionData.externalFunc) {
                    outStr << "(external)";
                    outStr << (void*)functionData.externalFunc;
                }
                break;

            case DERPTYPE_CUSTOMDATA:
                outStr << "<customData:" << customData << ">" << endl;
                break;

                // TODO: Modify user data class for serialization junk
                // and call that here.

                // TODO: Functions.

                // TODO: External functions.

            default:
                outStr << "???";
                break;
        }

        return outStr.str();
    }

    // ----------------------------------------------------------------------
    // Reference stuff
    // ----------------------------------------------------------------------

    DerpObject::Ref::Ref(const DerpObject::Ref &ref) {

        // // TODO: Remove this.
        // refCounter++;

        ob = ref.getPtr();
        if(ob) {
            ob->externalRefCount++;

            // TODO: Make normal error? Ref counter wrapped around.
            assert(ob->externalRefCount);
        }
    }

    DerpObject::Ref &DerpObject::Ref::operator=(const DerpObject::Ref &ref) {

        removeRef();

        ob = ref.getPtr();
        if(ob) {
            ob->externalRefCount++;

            // TODO: Make normal error? Ref counter wrapped around.
            assert(ob->externalRefCount);
        }
        return *this;
    }

    void DerpObject::Ref::removeRef(void) {

        if(ob) {

            ob->externalRefCount--;

            // The VM keeps one reference counter. This is to lazily clean
            // up refs after the VM has shutdown.

            // FIXME: This won't work with functions! Nothing will hold a
            // reference to any literal values.
            if(!ob->externalRefCount) {
                assert(!ob->vm);
                delete ob;
            }
        }
    }


    DerpObject::Ref::Ref(DerpObject *ob) {

        // // TODO: Remove this.
        // refCounter++;

        this->ob = ob;
        if(ob) {
            ob->externalRefCount++;

            // TODO: Make normal error? Ref counter wrapped around.
            assert(ob->externalRefCount);
        }
    }

    DerpObject::Ref::~Ref(void) {

        // refCounter--;

        removeRef();
    }

    DerpObject *DerpObject::Ref::getPtr(void) const {
        return ob;
    }

    DerpObject *DerpObject::Ref::operator->(void) {
        return ob;
    }

    const DerpObject *DerpObject::Ref::operator->(void) const {
        return ob;
    }

    DerpObject::Ref::Ref(void) {

        // // TODO: Remove this.
        // refCounter++;

        ob = NULL;
    }

    void DerpObject::Ref::reassign(DerpObject *ob) {
        removeRef();

        this->ob = ob;
        if(ob) {
            ob->externalRefCount++;

            // TODO: Make normal error? Ref counter wrapped around.
            assert(ob->externalRefCount);
        }
    }

    unsigned int DerpObject::getExternalRefCount(void) const {
        return externalRefCount;
    }

    bool DerpObject::getConst(void) const {
        return isConst;
    }

    void DerpObject::setConst(bool isConst) {
        this->isConst = isConst;
    }

    bool DerpObject::getCopyable(void) const {
        return isCopyable;
    }

    void DerpObject::setCopyable(bool isCopyable) {
        this->isCopyable = isCopyable;
    }

    DerpBasicType DerpObject::getType(void) const {
        return type;
    }

    DerpObject::Ref DerpObject::evalFunction(
        DerpContext *ctx,
        vector<DerpObject::Ref> *params,
        void *userData,
        DerpErrorState &errorState,
        bool dontPushContext,
        unsigned int stackDepth) {

        if(type != DERPTYPE_FUNCTION) {
            errorState.addError("Tried to call something as a function that is not a function.");
            return NULL;
        }

        // Fill in empty context with the global context.
        if(!ctx) {
            ctx = &vm->globalContext;
        }

        // Give us some valid object for parameters if no list was passed
        // in.
        vector<DerpObject::Ref> localParams;
        if(!params) {
            params = &localParams;
        }

        // Make a new context for this call.
        // DerpContext newContext(ctx); // Dynamic scoping if we use this line instead of the next.
        DerpContext newContext(vm->globalContext);

        if(!dontPushContext) {
            ctx = &newContext;
        }

        if(functionData.externalFunc) {

            // External function call.

            ExternalFuncData passData;
            passData.vm = vm;
            passData.context = ctx;
            passData.parameters = *params;
            passData.userData = userData;
            passData.errorState = &errorState;
            passData.stackDepth = stackDepth + 1;

            DerpObject::Ref ret = functionData.externalFunc(
                passData);

            return ret;

        } else {

            // Normal function call.

            // Match up input paramteres to parameter names and set them
            // in the context...

            unsigned int numInputParams = params ? params->size() : 0;
            unsigned int numFuncParams = functionData.paramNames ? functionData.paramNames->size() : 0;

            // Check number of parameters.
            if(numInputParams != numFuncParams) {
                errorState.addError("Incorrect number of parameters.");
                return NULL;
            }

            // Set function parameter variables in the context.
            for(unsigned int i = 0; i < numInputParams; i++) {
                DerpObject::Ref param = (*params)[i];
                ctx->setVariable((*functionData.paramNames)[i], param);
            }
        }

        functionData.callCounter++;

        // Check for call stack wrapping around. This isn't likely to
        // happen. But it might.

        if(!functionData.callCounter ||
           functionData.callCounter > DERP_MAX_STACK_FRAMES) {
            errorState.addError("Too much recursion on a single function.");
            functionData.callCounter--;
            return NULL;
        }

        DerpReturnType localReturnType = DERPRETURN_NORMAL;

        DerpObject::Ref ret = functionData.execNode->eval(
            ctx, &localReturnType,
            errorState, userData,
            stackDepth + 1);

        functionData.callCounter--;

        // Some error happened up the stack.
        if(localReturnType == DERPRETURN_ERROR) {
            return NULL;
        }

        return ret;
    }

    bool DerpObject::operator<(const DerpObject &ob) const {

        // Different types? Just use position in the type enum.
        if(ob.type != type) return type < ob.type;

        switch(type) {
            case DERPTYPE_FLOAT:
                return floatVal < ob.floatVal;
            case DERPTYPE_INT:
                return intVal < ob.intVal;
            case DERPTYPE_STRING:
                return *strVal < *ob.strVal;

                // TODO: Tables.
                // TODO: User data.
                // TODO: Functions.
                // TODO: External functions.

            default:
                // Unimplemented?
                assert(0);
        }
    }

    DerpObject::Ref::operator bool(void) {
        return bool(getPtr());
    }

    static const struct {
        DerpBasicType type;
        const char *str;
    } derpObjectTypeToStringMappings[] = {
        { DERPTYPE_STRING,     "string" },
        { DERPTYPE_FLOAT,      "float" },
        { DERPTYPE_INT,        "integer" },
        { DERPTYPE_TABLE,      "table" },
        { DERPTYPE_FUNCTION,   "function" },
        { DERPTYPE_CUSTOMDATA, "custom" },
        { DERPTYPE_NONE,       0 },
    };

    std::string derpObjectTypeToString(DerpBasicType t) {
        unsigned int i = 0;
        while(derpObjectTypeToStringMappings[i].str) {

            if(t == derpObjectTypeToStringMappings[i].type) {
                return derpObjectTypeToStringMappings[i].str;
            }

            i++;
        }
        return "unknown";
    }

    DerpObject::CustomData::CustomData(void) {
        deleteMeWhenDone = true;
    }

    DerpObject::CustomData::~CustomData(void) {
    }

    void DerpObject::CustomData::onLastReferenceGone(void) {
    }
}

