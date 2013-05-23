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

#pragma once

#include <iostream>
#include <cassert>

#include "derpconfig.h"
#include "derperror.h"
#include "derpvm.h"
#include "derpcontext.h"
#include "derpexecnode.h"
#include "derpobject.h"
#include "pooledstring.h"

namespace ExPop {

  #define CHECK_CHILDREN(x) if(children.size() != x) {                  \
        errorState.setFileAndLine(fileName, lineNumber);                \
        errorState.addError("Internal error. Expected more child nodes."); \
        *returnType = DERPRETURN_ERROR;                                 \
        return NULL;                                                    \
    }

    bool derpExec_checkObjectType(
        DerpBasicType t, DerpObject::Ref r,
        DerpErrorState &errorState, DerpExecNode *node,
        DerpReturnType *returnType);

  #define CHECK_ERROR() \
    (*returnType == DERPRETURN_ERROR ? true : false)

  #define CHECK_TYPE(type, ob) \
    derpExec_checkObjectType(type, ob, errorState, this, returnType)

  #define CHECK_TYPES_MATCH(ob1, ob2) \
    derpExec_checkObjectType(ob1->type, ob2, errorState, this, returnType)

  #define FLAG_ERROR(str)                                   \
    {                                                       \
        errorState.setFileAndLine(fileName, lineNumber);    \
        errorState.addError(str);                           \
        if(returnType) *returnType = DERPRETURN_ERROR;      \
    }

  #define TYPE_NOT_SUPPORTED(op) \
    FLAG_ERROR(derpSprintf("Type %s is not compatible with %s operator.", derpObjectTypeToString(obL->type).c_str(), op))

  #define EVAL_PARAMS_DECL                      \
    DerpContext *context,                       \
        DerpReturnType *returnType,             \
        DerpErrorState &errorState,             \
        void *userData

  #define EVAL_PARAMS_PASS                      \
    context, returnType, errorState, userData
}

