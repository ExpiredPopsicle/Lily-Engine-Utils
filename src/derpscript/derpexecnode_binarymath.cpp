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


