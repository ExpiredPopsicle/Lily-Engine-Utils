#pragma once

#include <vector>

#include "derpcontext.h"
#include "derpobject.h"
#include "derperror.h"
#include "pooledstring.h"

namespace ExPop {

    /// DerpVM represents the whole state of a scripting system. It
    /// can be directly instantiated, and most other public-facing
    /// objects are created in some way through it.
    class DerpVM {
    public:

        DerpVM(void);
        ~DerpVM(void);

        /// Run the garbage collector. This should be safe to run
        /// outside of internal VM systems. Some points in VM code
        /// might be unsafe to run this. External function calls
        /// should be fine.
        void garbageCollect(void);

        /// Runs a garbage collection only if the number of objects
        /// has exceeded the garbage collection threshold. Adjusts the
        /// threshold afterwards.
        void garbageCollectWithThreshold(void);

        /// Get the global variable context.
        DerpContext *getGlobalContext(void);

        /// Parse and evaluate a string. fileName is used for error
        /// reporting. If this returns a reference to NULL, it
        /// indicates an error. errorState should be checked for an
        /// explanation.
        DerpObject::Ref evalString(
            const std::string &str,
            DerpErrorState &errorState,
            const std::string &fileName);

        /// Compile a string and return a zero-parameter function
        /// object for it.
        DerpObject::Ref compileString(
            const std::string &str,
            DerpErrorState &errorState,
            const std::string &fileName);

        /// Make an object inside the VM. Make sure you set it to
        /// something before you assign it to some variable or return
        /// it. The default value isn't valid for much of anything.
        DerpObject::Ref makeObject(void);

        /// Get a pooled string reference to a filename.
        PooledString::Ref getFilenameRef(
            const std::string &fileName);

        /// Get the number of objects referring to a certain piece of
        /// user data.
        unsigned int getNumCustomDataRefs(DerpObject::CustomData *customData);

        /// Get the number of objects currently in the VM. Note that
        /// this does include objects that would be cleaned out by the
        /// garbage collector. To get an accurate count, run
        /// garbageCollect() before calling this.
        unsigned int getNumObjects(void);

    private:

        // All objects allocated for this VM go in here so we can
        // garbage collect them.
        std::vector<DerpObject*> gcObjects;

        // Check to make sure we haven't exceeded the configured
        // object count.
        bool checkObjectCount(void);

        // Numbers of references for custom data for quick lookups.
        // ie: Check to see if we still have internal references to it
        // before freeing it, or something.
        std::map<DerpObject::CustomData*, unsigned int> customDataRefs;

        // Increment and decrement user data reference counters. This
        // is called by DerpObject when the custom data is assigned.
        void addCustomDataRef(DerpObject::CustomData *customData);
        void removeCustomDataRef(DerpObject::CustomData *customData);

        void registerObject(DerpObject *ob);
        void unregisterObject(DerpObject *ob);

        // UGLY HACK for the garbage collector. Adds i to the external
        // reference count of all objects for each table referencing
        // it. i can be negative. This is used to neutralize internal
        // table references when trying to determine if we have actual
        // external references.
        void addTableRefs(int i);

        // For referring to registerObject, unregisterObject,
        // addCustomDataRef, removeCustomDataRef, and filenamePool.
        friend class DerpObject;

        // For calling checkObjectCount() from one of our internal
        // types.
        friend class DerpExecNode;

        unsigned int lastGCPass;

        // Global variables end up in this. Child of internalContext.
        DerpContext globalContext;

        // Internal stuff ends up in this. Not accessible outside the
        // VM unless they get it through globalContext.
        DerpContext internalContext;

        unsigned int objectCountGcThreshold;

        // Filenames are stored in a stringpool so that we can have
        // the filename attached to every single execution node
        // without using tons of memory.
        StringPool filenamePool;
    };
}
