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




