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

    DerpObject::Ref DerpExecNode::eval_binaryMathOp(
        DerpContext *context,
        DerpReturnType *returnType,
        DerpErrorState &errorState,
        void *userData) {

        CHECK_CHILDREN(2);

        DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);
        if(CHECK_ERROR()) {
            return NULL;
        }

        DerpObject::Ref obR = children[1]->eval(EVAL_PARAMS_PASS);
        if(CHECK_ERROR()) {
            return NULL;
        }

        if(!CHECK_TYPES_MATCH(obL, obR)) {
            return NULL;
        }

        DerpObject::Ref newOb(new DerpObject(vm));

        switch(type) {

            // Addition.
            case DERPEXEC_ADD:
                switch(obL->type) {
                    case DERPTYPE_STRING: newOb->setString(*obL->strVal + *obR->strVal); break;
                    case DERPTYPE_INT:    newOb->setInt(obL->intVal + obR->intVal); break;
                    case DERPTYPE_FLOAT:  newOb->setFloat(obL->floatVal + obR->floatVal); break;
                        // TODO: Array concatenation.
                        // TODO: Table combining.
                    default:
                        TYPE_NOT_SUPPORTED("addition");
                        return NULL;
                }
                break;

                // Subtraction.
            case DERPEXEC_SUBTRACT:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal - obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setFloat(obL->floatVal - obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("subtraction");
                        return NULL;
                }
                break;

                // Multiplication.
            case DERPEXEC_MULTIPLY:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal * obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setFloat(obL->floatVal * obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("multiplication");
                        return NULL;
                }
                break;

                // Division.
            case DERPEXEC_DIVIDE:
                switch(obL->type) {
                    case DERPTYPE_INT: {
                        // Integers need protection from division by zero.
                        if(obR->intVal == 0) {
                            FLAG_ERROR("Attempted integer divide by zero.");
                            return NULL;
                        }
                        newOb->setInt(obL->intVal / obR->intVal);
                    } break;
                    case DERPTYPE_FLOAT: newOb->setFloat(obL->floatVal / obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("division");
                        return NULL;
                }
                break;

                // Greater-Than.
            case DERPEXEC_GT:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal > obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal > obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("greater-than");
                        return NULL;
                }
                break;

                // Less-Than.
            case DERPEXEC_LT:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal < obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal < obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("less-than");
                        return NULL;
                }
                break;

                // Greater-Than or Equal.
            case DERPEXEC_GE:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal >= obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal >= obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("greater-than or equal");
                        return NULL;
                }
                break;

                // Less-Than or Equal.
            case DERPEXEC_LE:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal <= obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal <= obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("less-than or equal");
                        return NULL;
                }
                break;

                // Equality test.
            case DERPEXEC_EQ:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal == obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal == obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("equality");
                        return NULL;
                }
                break;

                // Not Equality test.
            case DERPEXEC_NEQ:
                switch(obL->type) {
                    case DERPTYPE_INT:   newOb->setInt(obL->intVal != obR->intVal); break;
                    case DERPTYPE_FLOAT: newOb->setInt(obL->floatVal != obR->floatVal); break;
                    default:
                        TYPE_NOT_SUPPORTED("non-equality");
                        return NULL;
                }
                break;

            default:
                FLAG_ERROR("FIXME: Unsupported opcode.");
                return NULL;
        }

        return newOb;

    }
}


