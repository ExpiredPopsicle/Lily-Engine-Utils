#pragma once

#include <map>
#include <string>

#include "derpobject.h"

namespace ExPop {

    /// DerpContext represents the current scope and is where the entire
    /// VM-side concept of variables lives.
    class DerpContext {
    public:

        DerpContext(void);
        DerpContext(DerpContext *parent);

        /// Set (and create if needed) a variable to some DerpObject.
        void setVariable(const std::string &name, DerpObject::Ref data);

        /// Clear out a variable. Doesn't clear protected status.
        void unsetVariable(const std::string &name);

        /// Get a variable by name. Will recurse if necessary to the
        /// parent context.
        DerpObject::Ref getVariable(const std::string &name);

        /// Set a variable to protected (or unprotected). Protected means
        /// that the reference cannot be reassigned. NOTE: Protected
        /// variables can be modified. They just can't be pointed to
        /// anything else. Use with const stuff to make them unmodifyable
        /// by scripts.
        void setVariableProtected(const std::string &name, bool refProtected);

        /// Get protected flag for a variable.
        bool getVariableProtected(const std::string &name);

        /// Clear all variables and protected status.
        void clearAllVariables(void);

    private:

        // Get the pointer to a variable reference. This can be used to
        // reassign the reference to something else later. NOTE: Only
        // guaranteed to be valid immediately after the call to this
        // function!
        DerpObject::Ref *getVariablePtr(
            const std::string &name,
            bool noRecurse = false);

        friend class DerpObject;    // For getVariablePtr().
        friend class DerpExecNode;  // For getVariablePtr().

        DerpContext *parent;

        std::map<std::string, DerpObject::Ref> variables;
        std::map<std::string, bool> variablesProtected;

    };
}

