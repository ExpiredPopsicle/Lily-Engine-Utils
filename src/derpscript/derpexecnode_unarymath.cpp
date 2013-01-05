#include "derpexecnode_internal.h"
using namespace std;

namespace ExPop {

    DerpObject::Ref DerpExecNode::eval_unaryMathOp(
        DerpContext *context,
        DerpReturnType *returnType,
        DerpErrorState &errorState,
        void *userData) {

        CHECK_CHILDREN(1);

        DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);

        if(CHECK_ERROR()) {
            return NULL;
        }

        switch(type) {

            // Increment.
            case DERPEXEC_INCREMENT:
                switch(obL->type) {
                    case DERPTYPE_INT: obL->intVal++; return obL;
                    case DERPTYPE_FLOAT: obL->floatVal++; return obL;
                    default:
                        TYPE_NOT_SUPPORTED("increment");
                        return NULL;
                }

                // Decrement.
            case DERPEXEC_DECREMENT:
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

