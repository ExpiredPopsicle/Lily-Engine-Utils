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
#include <cassert>
using namespace std;

#include "derpexecnode.h"
#include "derpobject.h"
#include "derpvm.h"
#include "derpparser.h"
#include "derpconfig.h"
#include "derpvm_builtinfunctions.h"

namespace ExPop {

    // ----------------------------------------------------------------------
    //  DerpVM implementation
    // ----------------------------------------------------------------------

  #define GARBAGECOLLECT_MIN_THRESHOLD 2048

    DerpVM::DerpVM(void) : globalContext(&internalContext), internalContext() {
        lastGCPass = 0;
        objectCountGcThreshold = GARBAGECOLLECT_MIN_THRESHOLD;

        derpVM_registerInternalFunctions(this, &internalContext);

        maxVmObs = 0;
    }

    DerpVM::~DerpVM(void) {

        // This will clean up global state except for things where
        // something still holds a reference.
        globalContext.clearAllVariables();
        garbageCollect();

        // Destroy all objects that have no external references.
        // Orphan everything else.

        while(gcObjects.size()) {

            if(gcObjects[0]->externalRefCount > 1) {
                unregisterObject(gcObjects[0]);
            } else {
                delete gcObjects[0];
            }
        }
    }

    void DerpVM::registerObject(DerpObject *ob) {

        assert(!ob->vm);
        ob->vm = this;

        gcObjects.push_back(ob);
        ob->vmGarbageCollectionListIndex = gcObjects.size() - 1;
        ob->externalRefCount++;

        // TODO: Make normal error? Ref counter wrapped around.
        assert(ob->externalRefCount);
    }

    void DerpVM::unregisterObject(DerpObject *ob) {

        assert(ob->vm == this);

        // There are too many ways for things like circular references
        // to make it outside of a VM, which is the only place where
        // they'll be properly handled. So I'm applying the bludgeon
        // of just nuking all non-trivial data when it leaves the VM.
        DerpBasicType dt = ob->getType();
        if(dt == DERPTYPE_STRING || dt == DERPTYPE_FLOAT || dt == DERPTYPE_INT) {
        } else {
            ob->clearData();
        }

        // Move the end item into this one's slot.
        gcObjects[ob->vmGarbageCollectionListIndex] = gcObjects[gcObjects.size() - 1];
        gcObjects[ob->vmGarbageCollectionListIndex]->vmGarbageCollectionListIndex = ob->vmGarbageCollectionListIndex;

        // Erase the duplicate at the end.
        gcObjects.erase(gcObjects.end() - 1);

        ob->vm = NULL;
        ob->vmGarbageCollectionListIndex = 0;
        ob->externalRefCount--;

        // TODO: Maybe just clear out everything related to the object
        // to make sure there's nothing pointing back into the VM.
        // Strings and numbers might be okay, but having those and not
        // functions, arrays, or tables, seems to be an inconsistency
        // in the handling of the loose data.

    }

    void DerpVM::addCustomDataRef(DerpObject::CustomData *customData) {

        customDataRefs[customData]++;

        // TODO: Make less fatal?
        assert(customDataRefs[customData]);
    }

    void DerpVM::removeCustomDataRef(DerpObject::CustomData *customData) {

        assert(customDataRefs[customData]);

        customDataRefs[customData]--;

        if(!customDataRefs[customData]) {
            customDataRefs.erase(customData);

            customData->onLastReferenceGone();

            if(customData->deleteMeWhenDone) {
                delete customData;
            }
        }

    }

    unsigned int DerpVM::getNumCustomDataRefs(DerpObject::CustomData *customData) {
        if(customDataRefs.count(customData)) {
            return customDataRefs[customData];
        }
        return 0;
    }

    void DerpVM::addTableRefs(int amount) {

        // This function is sort of a hack. It goes through and
        // nullifies the reference counts from table references. Also
        // used to restore those values afterwards. BE WARNED THAT THE
        // REFERENCE COUNTS ARE IN A WEIRD STATE AFTER THIS.

        // FIXME: A slower alternative to this state meddling might
        // just build a map of table reference counts by pointer that
        // we can then mess with later.

        for(unsigned int i = 0; i < gcObjects.size(); i++) {
            if(gcObjects[i]->getType() == DERPTYPE_TABLE) {
                std::map<DerpObject::Ref, DerpObject::Ref, DerpObjectCompare>::iterator mapIter;
                for(mapIter = gcObjects[i]->tableData->begin(); mapIter != gcObjects[i]->tableData->end(); mapIter++) {
                    (*mapIter).first.getPtr()->externalRefCount += amount;
                    (*mapIter).second.getPtr()->externalRefCount += amount;
                }
            }
        }
    }

    void DerpVM::garbageCollect(void) {

        if(gcObjects.size() > maxVmObs) {
            maxVmObs = gcObjects.size();
        }

        // Do something about reference counts on tables, otherwise
        // circular references will own us.
        addTableRefs(-1);

        lastGCPass++;

        // Mark all objects with external references, and all
        // functions will positive call counts.
        for(unsigned int i = 0; i < gcObjects.size(); i++) {
            if(gcObjects[i]->externalRefCount > 1) {

                // Object has external references. Mark it.
                gcObjects[i]->markGCPass(lastGCPass);

            } else if(gcObjects[i]->type == DERPTYPE_FUNCTION) {
                if(gcObjects[i]->functionData.callCounter) {

                    // Function is in the callstack. Mark it as in
                    // use.
                    gcObjects[i]->markGCPass(lastGCPass);
                }
            }
        }

        vector<DerpObject*> obsToKill;
        vector<DerpObject*> obsToDelete;

        // Clean out anything that didn't make the cut.
        for(unsigned int i = 0; i < gcObjects.size(); i++) {
            if(gcObjects[i]->lastGCPass != lastGCPass) {

                // Didn't get hit on the last garbage collection pass.
                // Add it to the list of stuff to clean up.
                obsToKill.push_back(gcObjects[i]);

                if(gcObjects[i]->externalRefCount == 1) {

                    // This object will have to be manually deleted by
                    // us because the only reference it has is the
                    // fake one the VM adds.
                    obsToDelete.push_back(gcObjects[i]);
                }
            }
        }

        // Restore table ref counts.
        addTableRefs(1);

        // Complicated method to clean up data. First, we're going to
        // hold references to everything that we want to delete to
        // tmpRefs. Then we're going to unregister them all from the
        // VM. Then we'll call clearData on each of them to "unstick"
        // them from each other (they won't have refs to each other
        // that'll cause some to be destroyed leaving a dangling
        // pointer int the list of stuff to destroy). After that, the
        // only reference to each object will be in the tmpRefs, and
        // they'll be independent. Letting tmpRefs get cleared will
        // release all those references.

        // Hold references for everything we want to delete.
        vector<DerpObject::Ref> tmpRefs;
        for(unsigned int i = 0; i < obsToDelete.size(); i++) {
            tmpRefs.push_back(obsToDelete[i]);
        }

        // Unregister everything that didn't get touched by the
        // garbage collector. Will not trigger auto destruction for
        // last reference because we hold tmpRefs.
        for(unsigned int i = 0; i < obsToKill.size(); i++) {
            DerpObject *ob = obsToKill[i];
            unregisterObject(ob);
        }

        // Unstick all the to-be-deleted data.
        for(unsigned int i = 0; i < obsToDelete.size(); i++) {
            obsToDelete[i]->clearData();
        }

        // Nuke the now independent data.
        tmpRefs.clear();
    }

    DerpContext *DerpVM::getGlobalContext(void) {
        return &globalContext;
    }

    DerpObject::Ref DerpVM::evalString(
        const std::string &str,
        DerpErrorState &errorState,
        const std::string &fileName) {

        DerpObject::Ref fakeFunctionObject = compileString(
            str,
            errorState,
            fileName);

        if(!fakeFunctionObject) {
            return NULL;
        }

        DerpObject::Ref ret = fakeFunctionObject->evalFunction(
            &globalContext,
            NULL,
            NULL,
            errorState,
            true);

        return ret;
    }

    DerpObject::Ref DerpVM::compileString(
        const std::string &str,
        DerpErrorState &errorState,
        const std::string &fileName) {

        DerpExecNode *node = derpParseText(
            this, str, errorState, fileName);

        if(!node) {
            return DerpObject::Ref(NULL);
        }

        DerpObject::Ref functionObject = makeObject();
        vector<string> *paramNames = new vector<string>;
        functionObject->setFunction(node, paramNames);

        return functionObject;
    }

    DerpObject::Ref DerpVM::makeObject(void) {
        return DerpObject::Ref(new DerpObject(this));
    }

    static unsigned int higherPow2(unsigned int i) {
        unsigned int j = 1;
        while(j < i) j <<= 1;
        return j;
    }

    void DerpVM::garbageCollectWithThreshold(void) {

        if(gcObjects.size() > objectCountGcThreshold) {

            garbageCollect();

            // Readjust the object count threshold.
            objectCountGcThreshold = higherPow2(gcObjects.size());
            if(objectCountGcThreshold <= GARBAGECOLLECT_MIN_THRESHOLD) {
                objectCountGcThreshold = GARBAGECOLLECT_MIN_THRESHOLD;
            }
        }
    }

    bool DerpVM::checkObjectCount(void) {
        return (gcObjects.size() <= DERP_MAX_OBJECT_COUNT);
    }

    PooledString::Ref DerpVM::getFilenameRef(const std::string &fileName) {
        return filenamePool.getOrAdd(fileName);
    }

    unsigned int DerpVM::getNumObjects(void) {
        return gcObjects.size();
    }
}
