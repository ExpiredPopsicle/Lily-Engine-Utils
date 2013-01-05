#include "derpexecnode_internal.h"
using namespace std;

namespace ExPop {

    DerpObject::Ref DerpExecNode::eval_loop(
        DerpContext *context,
        DerpReturnType *returnType,
        DerpErrorState &errorState,
        void *userData) {

        DerpContext childContext(context);
        context = &childContext;

        DerpObject::Ref condition(NULL);
        DerpObject::Ref ret;

        // For all loops...
        //   children[0] is the loop action to execute.

        // For while loops...
        //   children[1] is the loop PRE condition.

        // For do/while loops...
        //   children[2] is the loop POST condition.

        // For "for" loops...
        //   children[3] is the loop init.
        //   children[4] is the loop iteration action.


        // Run loop init for for loops.
        if(children.size() >= 4 && children[3]) {
            DerpObject::Ref initResult = children[3]->eval(EVAL_PARAMS_PASS);

            // Error in initResult.
            if(CHECK_ERROR() || !initResult.getPtr()) {
                return NULL;
            }
        }


        while(1) {

            // Check loop starting condition.
            if(children.size() >= 2 && children[1]) {
                condition = children[1]->eval(EVAL_PARAMS_PASS);

                // Error in condition.
                if(CHECK_ERROR() || !condition.getPtr()) {
                    return NULL;
                }

                // Bad type for condition.
                if(condition->type != DERPTYPE_INT) {
                    FLAG_ERROR("Incorrect type for loop pre-condition.");
                    return NULL;
                }

                // Now actually check condition.
                if(!condition->intVal) {
                    ret = DerpObject::Ref(new DerpObject(vm));
                    ret->setInt(0);
                    return ret;
                }
            }


            // Do the loop's action.
            if(children.size() >= 1 && children[0]) {
                ret = children[0]->eval(EVAL_PARAMS_PASS);

                // Handle break or return from inside the loop.
                if(*returnType != DERPRETURN_NORMAL) {

                    if(*returnType == DERPRETURN_BREAK) {

                        // Eat break statements.
                        *returnType = DERPRETURN_NORMAL;
                        return ret;

                    } else if(*returnType == DERPRETURN_CONTINUE) {

                        // Eat continue statements. (But don't return.)
                        *returnType = DERPRETURN_NORMAL;

                    } else if(*returnType == DERPRETURN_ERROR) {

                        // Error. Just bail out with NULL.
                        return NULL;

                    } else {

                        // I don't know what this is, but it wasn't a
                        // normal return. So just bail out. (Probably
                        // function return.)
                        return ret;
                    }
                }

            }

            // Run loop init for for loops.
            if(children.size() >= 5 && children[4]) {
                DerpObject::Ref iterateResult = children[4]->eval(EVAL_PARAMS_PASS);

                // Error in initResult.
                if(CHECK_ERROR() || !iterateResult.getPtr()) {
                    return NULL;
                }
            }


            // Check loop ending condition.
            if(children.size() >= 3 && children[2]) {
                condition = children[2]->eval(EVAL_PARAMS_PASS);

                // Error in condition.
                if(CHECK_ERROR() || !condition.getPtr()) {
                    return NULL;
                }

                // Bad type for condition.
                if(condition->type != DERPTYPE_INT) {
                    FLAG_ERROR("Incorrect type for loop post-condition.");
                    return NULL;
                }

                // Now actually check condition.
                if(!condition->intVal) {
                    ret = DerpObject::Ref(new DerpObject(vm));
                    ret->setInt(0);
                    return ret;
                }

            }
        }

        return ret;

    }
}

