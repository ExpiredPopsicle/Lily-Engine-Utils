#include "derpcontext.h"

namespace ExPop {

    DerpContext::DerpContext(void) {
        parent = NULL;
    }

    DerpContext::DerpContext(DerpContext *parent) {
        this->parent = parent;
    }

    void DerpContext::setVariable(const std::string &name, DerpObject::Ref data) {

        // TODO: Check total visible variable count to make sure we
        // don't go over some arbitrary limit.

        variables[name] = data;
    }


    DerpObject::Ref DerpContext::getVariable(const std::string &name) {

        if(!variables.count(name)) {
            if(parent) {
                return parent->getVariable(name);
            } else {
                return NULL;
            }
        }

        return variables[name];
    }

    DerpObject::Ref *DerpContext::getVariablePtr(const std::string &name, bool noRecurse) {

        if(getVariableProtected(name)) {
            return NULL;
        }

        if(!variables.count(name)) {
            if(!noRecurse && parent) {
                return parent->getVariablePtr(name);
            } else {
                return NULL;
            }
        }

        return &(variables[name]);
    }

    void DerpContext::unsetVariable(const std::string &name) {
        variables.erase(name);
    }

    void DerpContext::setVariableProtected(const std::string &name, bool refProtected) {
        if(refProtected) {
            variablesProtected[name] = true;
        } else {
            variablesProtected.erase(name);
        }
    }

    bool DerpContext::getVariableProtected(const std::string &name) {
        return variablesProtected.count(name);
    }

    void DerpContext::clearAllVariables(void) {
        variables.clear();
        variablesProtected.clear();
    }

}

