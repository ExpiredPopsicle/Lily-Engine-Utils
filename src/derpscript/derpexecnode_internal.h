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

