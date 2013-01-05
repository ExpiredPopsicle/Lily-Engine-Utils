#include "derpexecnode_internal.h"
using namespace std;

namespace ExPop {

    DerpObject::Ref DerpExecNode::eval_functionCall(
        DerpContext *context,
        DerpReturnType *returnType,
        DerpErrorState &errorState,
        void *userData) {

        // Check that we at least have the function lookup node as a
        // child.
        if(!children.size()) {
            FLAG_ERROR("Internal error. No child nodes on function call.");
            return NULL;
        }

        // Eval the function lookup itself and make sure it's a
        // function.
        DerpObject::Ref functionToCall = children[0]->eval(EVAL_PARAMS_PASS);
        if(!CHECK_TYPE(DERPTYPE_FUNCTION, functionToCall) || CHECK_ERROR()) {
            return NULL;
        }

        // Set function parameters.
        vector<DerpObject::Ref> params;
        for(unsigned int i = 1; i < children.size(); i++) {
            DerpObject::Ref param = children[i]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) return NULL;
            params.push_back(param);
        }

        DerpObject::Ref ret = functionToCall->evalFunction(
            context, &params, userData, errorState);

        if(!ret.getPtr()) {
            if(returnType) *returnType = DERPRETURN_ERROR;
            return NULL;
        }

        return ret;

    }
}




