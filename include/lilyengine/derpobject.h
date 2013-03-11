#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <map>

namespace ExPop {

    /// Basic data types.
    enum DerpBasicType {
        DERPTYPE_STRING,
        DERPTYPE_FLOAT,
        DERPTYPE_INT,
        DERPTYPE_TABLE,
        DERPTYPE_FUNCTION,
        DERPTYPE_CUSTOMDATA,
        DERPTYPE_NONE
    };

    class DerpVM;
    class DerpExecNode;
    class DerpContext;
    class DerpErrorState;
    class DerpObject;
    class DerpObjectCompare;

    /// DerpObject is a public-facing type that represents data inside
    /// of a VM.
    class DerpObject {
    public:

        DerpObject(DerpVM *vm);
        ~DerpObject(void);

        class Ref;
        class ExternalFuncData;
        class CustomData;

        /// C++ function pointer type.
        typedef Ref (*ExternalFunction)(
            ExternalFuncData &data);

        /// Set this to NULL and clear any allocated data.
        void clearData(void);

        /// Set a string value. Also sets the type to DERPTYPE_STRING.
        void setString(const std::string &str);

        /// Get string value. MUST BE A STRING. (Use getType().)
        std::string getString(void);

        /// Set integer value. Also sets the type to DERPTYPE_INT.
        void setInt(int i);

        /// Get integer value. MUST BE AN INTEGER. (Use getType().)
        int getInt(void);

        /// Set the object to a floating point value. Also sets the
        /// type to DERPTYPE_FLOAT.
        void setFloat(float f);

        /// Get the floating point value. MUST BE A FLOAT. (Use
        /// getType().)
        float getFloat(void);

        /// Internal function. (Unless you can get your hands on a
        /// DerpExecNode otherwise.) Set the function to some
        /// DerpExecNode tree. Note: This takes ownership of the node
        /// and the paramNames. Also sets the type to
        /// DERPTYPE_FUNCTION.
        void setFunction(
            DerpExecNode *node,
            std::vector<std::string> *paramNames);

        /// Set a C++ function for a function object. Also sets the
        /// type to DERPTYPE_FUNCTION.
        void setExternalFunction(ExternalFunction func);

        /// Clears value and turns this object into an empty table.
        void setTable(void);

        /// Sets an item in a table. Must already be a table!
        void setInTable(DerpObject::Ref key, DerpObject::Ref value);

        /// Clear a slot in a table.
        void clearInTable(DerpObject::Ref key);

        /// Get an object in a table.
        DerpObject::Ref getInTable(DerpObject::Ref key);

        /// Get a pointer to the ref that holds an object in a table.
        DerpObject::Ref *getInTablePtr(DerpObject::Ref key);

        /// Determine if a given type is valid for indexing into a
        /// table.
        static bool isValidKeyType(DerpBasicType t);

        /// Set a custom data object.
        void setCustomData(DerpObject::CustomData *customData);

        /// Get a custom data object.
        DerpObject::CustomData *getCustomData(void);

        /// Sets this object to match some other object.
        void set(DerpObject *otherOb);

        /// Copies this object. Really just makes a new object and
        /// calls set() internally.
        DerpObject *copy(void);

        /// Probably shouldn't use this for anything important (prone
        /// to change all the time), but if you want to have a peek at
        /// the state of the VM, this will help.
        std::string debugString(void) const;

        /// Get the number of DerpObject::Refs that point to this.
        unsigned int getExternalRefCount(void) const;

        /// Get const property.
        bool getConst(void) const;

        /// Set const property. When this is on, the value cannot be
        /// modified. It'll just throw an error if the program tries
        /// it.
        void setConst(bool isConst);

        /// Get the copyable property.
        bool getCopyable(void) const;

        /// Set the copyable property. When this is on, the object
        /// cannot be copied. Might be useful for ensuring custom data
        /// types only have a single object representing them. Should
        /// probably be used with the Const property.
        void setCopyable(bool isCopyable);

        /// Get the type of this DerpObject.
        DerpBasicType getType(void) const;

        /// Execute a function object. parameters may be NULL or an
        /// empty std::vector<DerpObject::Ref>to indicate no
        /// parameters. ctx may be NULL to just use the VM's global
        /// context.
        ///
        /// If dontPushContext is not set, this function will create a
        /// new context with a parent context of the one you pass in
        /// as ctx. If it is set, it will use ctx directly. This is
        /// necessary for the function's local variables to become
        /// variables in the context. (If you wanted to, say,
        /// implement the equivalent of #include to have stuff show up
        /// in the current scope or have your script define some stuff
        /// as globals.)
        ///
        /// NOTE: Any parameters you pass in will be defined in the
        /// selected context. If dontPushContext is set to true, the
        /// context that you pass in will be modified to hold these
        /// parameters variables.
        DerpObject::Ref evalFunction(
            DerpContext *ctx,
            std::vector<DerpObject::Ref> *parameters,
            void *userData,
            DerpErrorState &errorState,
            bool dontPushContext = false);

        /// Reference type. Counts references to DerpObjects so
        /// external stuff can hold onto stuff inside the VM.
        /// DerpContext objects also use these so we don't even have
        /// to worry about contexts during garbage collection.
        class Ref {
        public:
            Ref(void);
            Ref(const Ref &ref);
            Ref(DerpObject *ob);
            ~Ref(void);
            DerpObject *getPtr(void) const;
            DerpObject *operator->(void);
            const DerpObject *operator->(void) const;
            Ref &operator=(const Ref &ref);

            void reassign(DerpObject *ob);

            operator bool(void);
        private:
            void removeRef(void);
            DerpObject *ob;
        };

        /// External function data. Contains everything an external
        /// function call should need regarding context, parameters,
        /// user data, and error reporting.
        class ExternalFuncData {
        public:

            DerpVM *vm;
            DerpContext *context;
            std::vector<DerpObject::Ref> parameters;
            void *userData;
            DerpErrorState *errorState;
        };

        /// Derive custom data type objects from this. (Or just derive
        /// from this, add a field, and point it at whatever.)
        class CustomData {
        public:

            // TODO: Add serialization virtuals.

            CustomData(void);
            virtual ~CustomData(void);

        protected:

            /// Override this to add special handling for when the
            /// last reference to this data goes away.
            virtual void onLastReferenceGone(void);

            /// Set this to true to delete the object when the last
            /// reference to this custom data goes away. Defaults to
            /// true.
            bool deleteMeWhenDone;

        private:

            // DerpVM is responsible for cleaning this up if
            // necessary.
            friend class DerpVM;

        };

        bool operator<(const DerpObject &ob) const;

    private:

        DerpBasicType type;

        union {

            // Strings, floats, and integers.
            std::string *strVal;
            float floatVal;
            int intVal;

            // Function data.
            struct {
                DerpExecNode *execNode;
                unsigned int callCounter;
                std::vector<std::string> *paramNames;
                ExternalFunction externalFunc;
            } functionData;

            // Tables.

            // Note: There's a potential for circular references by
            // using external reference types as key and value in this
            // table. The garbage collector must compensate for this.
            std::map<DerpObject::Ref, DerpObject::Ref, DerpObjectCompare> *tableData;

            // User data.
            CustomData *customData;
        };

        DerpVM *vm;
        unsigned int externalRefCount;
        unsigned int vmGarbageCollectionListIndex;
        unsigned int lastGCPass;
        void markGCPass(unsigned int passNum);

        bool isConst;
        bool isCopyable;

        friend class DerpVM;
        friend class DerpObject::Ref;
        friend class DerpExecNode;

        // This is just a utility function used by debugString(). It
        // recurses through tables to make sure everything pointed to
        // by this table is shown.
        void dumpTable(
            std::ostream &ostr,
            std::map<const DerpObject*, bool> &stuffAlreadyDone,
            bool isRoot) const;
    };

    class DerpObjectCompare {
    public:
        bool operator()(const DerpObject::Ref &a, const DerpObject::Ref &b) const;
    };

    std::string derpObjectTypeToString(DerpBasicType t);
}

