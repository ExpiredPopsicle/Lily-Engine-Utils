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

    DerpObject::Ref DerpExecNode::eval_unaryMathOp(
        DerpContext *context,
        DerpReturnType *returnType,
        DerpErrorState &errorState,
        void *userData,
        unsigned int stackDepth) {

        CHECK_CHILDREN(1);

        DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);

        if(CHECK_ERROR()) {
            return NULL;
        }

        switch(type) {

            // Increment.
            case DERPEXEC_INCREMENT:
                if(obL->getConst()) {
                    FLAG_ERROR("Attempted to increment a constant.");
                    return NULL;
                }
                switch(obL->type) {
                    case DERPTYPE_INT: obL->intVal++; return obL;
                    case DERPTYPE_FLOAT: obL->floatVal++; return obL;
                    default:
                        TYPE_NOT_SUPPORTED("increment");
                        return NULL;
                }

            // Decrement.
            case DERPEXEC_DECREMENT:
                if(obL->getConst()) {
                    FLAG_ERROR("Attempted to decrement a constant.");
                    return NULL;
                }
                switch(obL->type) {
                    case DERPTYPE_INT: obL->intVal--; return obL;
                    case DERPTYPE_FLOAT: obL->floatVal--; return obL;
                    default:
                        TYPE_NOT_SUPPORTED("decrement");
                        return NULL;
                }

            // Not.
            case DERPEXEC_NOT: {
                DerpObject::Ref newOb(new DerpObject(vm));
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(!obL->intVal); return newOb;
                    case DERPTYPE_FLOAT: newOb->setInt(!obL->floatVal); return newOb;
                    default:
                        TYPE_NOT_SUPPORTED("decrement");
                        return NULL;
                }
            }

            default:
                FLAG_ERROR("FIXME: Unsupported opcode.");
                return NULL;
        }
    }
}

