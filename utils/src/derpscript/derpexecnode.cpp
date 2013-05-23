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

    DerpExecNode::DerpExecNode(
        DerpVM *vm,
        unsigned int lineNumber,
        const std::string &fileName) {

        type = DERPEXEC_ERROROP;
        data = NULL;

        this->vm = vm;
        this->lineNumber = lineNumber;
        // this->fileName = vm->filenamePool.getOrAdd(fileName);
        this->fileName = vm->getFilenameRef(fileName);
    }

    DerpExecNode::~DerpExecNode(void) {

        // Clean up child nodes.
        for(unsigned int i = 0; i < children.size(); i++) {
            delete children[i];
        }
    }

    DerpObject::Ref DerpExecNode::eval(
        EVAL_PARAMS_DECL) {

        errorState.setFileAndLine(
            fileName,
            lineNumber);

        vm->garbageCollectWithThreshold();

        if(!vm->checkObjectCount()) {
            FLAG_ERROR("Exceeded maximum object count in VM");
            return NULL;
        }

        DerpReturnType localReturnType = DERPRETURN_NORMAL;
        if(!returnType) returnType = &localReturnType;
        *returnType = DERPRETURN_NORMAL;

        // TODO: Every recursive call to eval() needs returnType and
        // errorOut! Maybe!

        // TODO: Arbitrary fake stack depth limit.

        // Automatically hook up to the global context if none other
        // exists.
        assert(vm);
        if(!context) {
            context = vm->getGlobalContext();
        }
        assert(context);

        switch(type) {
            case DERPEXEC_ADD:
            case DERPEXEC_SUBTRACT:
            case DERPEXEC_MULTIPLY:
            case DERPEXEC_DIVIDE:
            case DERPEXEC_AND:
            case DERPEXEC_OR:
            case DERPEXEC_GT:
            case DERPEXEC_LT:
            case DERPEXEC_GE:
            case DERPEXEC_LE:
            case DERPEXEC_EQ:
            case DERPEXEC_NEQ:
                return eval_binaryMathOp(EVAL_PARAMS_PASS);

            case DERPEXEC_INCREMENT:
            case DERPEXEC_DECREMENT:
            case DERPEXEC_NOT:
                return eval_unaryMathOp(EVAL_PARAMS_PASS);

            case DERPEXEC_LOOP:
                return eval_loop(EVAL_PARAMS_PASS);

            case DERPEXEC_FUNCTIONCALL:
                return eval_functionCall(EVAL_PARAMS_PASS);

            case DERPEXEC_LITERAL:
                return DerpObject::Ref(data);

            default:
                break;
        }

        if(type == DERPEXEC_VARLOOKUP) {

            // If this made it through the parser as something other than
            // a string, we have a problem in the parser.
            assert(data->type == DERPTYPE_STRING);

            DerpObject::Ref foundOb = context->getVariable(*data->strVal);

            if(foundOb.getPtr()) {
                return foundOb;
            }

            FLAG_ERROR(derpSprintf("Cannot find variable \"%s\".", data->strVal->c_str()));
            return NULL;

        } else if(type == DERPEXEC_ASSIGNMENT) {

            // Assignment operation

            CHECK_CHILDREN(2);

            DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpObject::Ref obR = children[1]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            if(obL->type == DERPTYPE_FUNCTION && obL->functionData.callCounter) {
                FLAG_ERROR("Attempting to modify a function that's being called.");
                return NULL;
            }

            if(obL->getConst()) {
                FLAG_ERROR("Attempting to modify a constant.");
                return NULL;
            }

            if(!obR->getCopyable()) {
                FLAG_ERROR("Attempting to copy a unique object.");
                return NULL;
            }

            obL->set(obR.getPtr());

            return obL;

        } else if(type == DERPEXEC_REFASSIGNMENT) {

            // Assignment operation, but just point to the new thing.

            CHECK_CHILDREN(2);

            DerpObject::Ref obR = children[1]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpObject::Ref *obL = children[0]->evalPtr(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            if(obL) {
                obL->reassign(obR.getPtr());
                return *obL;
            }

            // Should never happen.
            assert(0);

        } else if(
            type == DERPEXEC_BLOCK ||
            type == DERPEXEC_FREEBLOCK) {

            if(!children.size()) {
                // No children? Just return a zero.
                DerpObject::Ref newOb(new DerpObject(vm));
                newOb->setInt(0);
                return newOb;
            }

            DerpObject::Ref possibleRet;

            DerpContext childContext(context);
            DerpContext *contextForBlock =
                (type == DERPEXEC_FREEBLOCK) ? context : &childContext;

            for(unsigned int i = 0; i < children.size(); i++) {
                possibleRet = children[i]->eval(
                    contextForBlock, returnType,
                    errorState, userData);

                // Check to see if we got a "return", "break", or anything
                // that would cause us to prematurely end the block. If we
                // did, return from this evaluation with the same thing.
                // The loop or function call exec nodes will handle the
                // result.

                if(*returnType == DERPRETURN_FUNCTIONRETURN ||
                   *returnType == DERPRETURN_BREAK ||
                   *returnType == DERPRETURN_CONTINUE ||
                   *returnType == DERPRETURN_ERROR) break;
            }

            return possibleRet;

        } else if(type == DERPEXEC_RETURN) {

            CHECK_CHILDREN(1);

            DerpObject::Ref ret = children[0]->eval(EVAL_PARAMS_PASS);

            if(CHECK_ERROR()) {
                return NULL;
            }

            *returnType = DERPRETURN_FUNCTIONRETURN;
            return ret;

        } else if(type == DERPEXEC_VARIABLEDEC) {

            DerpObject::Ref *ret = evalPtr(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            return *ret;

        } else if(type == DERPEXEC_DEBUGPRINT) {

            // TODO: Remove this entire opcode.
            DerpObject::Ref ret = children[0]->eval(EVAL_PARAMS_PASS);
            if(ret.getPtr()) {
                cout << "DEBUG OUT: " << ret->debugString() << endl;
            }
            return ret;

        } else if(type == DERPEXEC_IFELSE) {

            DerpObject::Ref condition = children[0]->eval(EVAL_PARAMS_PASS);

            if(CHECK_ERROR() || !condition.getPtr()) {
                return NULL;
            }

            if(condition->type != DERPTYPE_INT) {
                FLAG_ERROR("Bad type for if statement condition.");
                return NULL;
            }

            if(condition->intVal) {
                return children[1]->eval(EVAL_PARAMS_PASS);
            } else if(children.size() > 2 && children[2]) {
                return children[2]->eval(EVAL_PARAMS_PASS);
            }

        } else if(type == DERPEXEC_BREAK) {

            DerpObject::Ref ret = DerpObject::Ref(new DerpObject(vm));
            ret->setInt(0);
            *returnType = DERPRETURN_BREAK;
            return ret;

        } else if(type == DERPEXEC_CONTINUE) {

            DerpObject::Ref ret = DerpObject::Ref(new DerpObject(vm));
            ret->setInt(0);
            *returnType = DERPRETURN_CONTINUE;
            return ret;

        } else if(type == DERPEXEC_INDEX) {

            CHECK_CHILDREN(2);

            DerpObject::Ref obR = children[1]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpBasicType dt = obL->getType();
            if(dt != DERPTYPE_TABLE) {
                FLAG_ERROR("Tried to index into something that is not a table.");
                return NULL;
            }

            // Make sure the key is something reasonable to use as a key.
            if(!obL->isValidKeyType(obR->getType())) {
                FLAG_ERROR(
                    derpSprintf(
                        "Tried to index into a %s with a %s.",
                        derpObjectTypeToString(obL->getType()).c_str(),
                        derpObjectTypeToString(obR->getType()).c_str()));
                return NULL;
            }

            // Make sure the key is copyable in case we have to make an
            // entry for it in the table.
            if(!obR->getCopyable()) {
                FLAG_ERROR(
                    derpSprintf(
                        "Tried to index into something with a non-copyable index."));
                return NULL;
            }

            DerpObject::Ref result = obL->getInTable(obR);

            if(!result) {
                if(!obL->getConst()) {
                    result = vm->makeObject();
                    result->setInt(0);
                    obL->setInTable(obR->copy(), result);
                } else {
                    FLAG_ERROR("Tried to access a non-existing entry in a const table. Cannot auto-create entry.");
                    return NULL;
                }
            }

            return result;

        } else {

            // TODO: Not yet implemented.
            assert(!"Unimplemented DerpExecNode type.");

        }

        // TODO: Should this be an error?
        return DerpObject::Ref((DerpObject*)NULL);
    }

    DerpObject::Ref *DerpExecNode::evalPtr(
        EVAL_PARAMS_DECL) {

        errorState.setFileAndLine(
            fileName,
            lineNumber);

        // Automatically hook up to the global context if none other
        // exists.
        assert(vm);
        if(!context) {
            context = vm->getGlobalContext();
        }

        if(type == DERPEXEC_VARLOOKUP) {

            // Make sure it's a string.
            if(data->type != DERPTYPE_STRING) {
                FLAG_ERROR("Non-string variable for reference lookup.");
                return NULL;
            }

            if(context->getVariableProtected(*data->strVal)) {
                FLAG_ERROR(
                    derpSprintf(
                        "Variable \"%s\" is protected and not valid for reference lookup.",
                        data->strVal->c_str()));
                return NULL;
            }

            // Do the lookup.
            DerpObject::Ref *foundOb = context->getVariablePtr(*data->strVal);
            if(foundOb) {
                return foundOb;
            }

            // Bad lookup.
            FLAG_ERROR(
                derpSprintf(
                    "Bad variable \"%s\" for reference lookup.",
                    data->strVal->c_str()));
            return NULL;

        } else if(type == DERPEXEC_VARIABLEDEC) {

            // Make sure it's a string.
            if(data->type != DERPTYPE_STRING) {
                FLAG_ERROR("Non-string variable for variable declaration.");
                return NULL;
            }

            // Make sure it doesn't exist in the current context.
            if(context->getVariablePtr(*data->strVal, true)) {
                FLAG_ERROR(
                    derpSprintf(
                        "Variable \"%s\" already exists.",
                        data->strVal->c_str()));
                return NULL;
            }

            // Set it in the context.
            DerpObject::Ref newOb(new DerpObject(vm));
            context->setVariable(*data->strVal, newOb);
            return context->getVariablePtr(*data->strVal);

        } else if(type == DERPEXEC_INDEX) {

            CHECK_CHILDREN(2);

            DerpObject::Ref obR = children[1]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpObject::Ref obL = children[0]->eval(EVAL_PARAMS_PASS);
            if(CHECK_ERROR()) {
                return NULL;
            }

            DerpBasicType dt = obL->getType();
            if(dt != DERPTYPE_TABLE) {
                FLAG_ERROR("Tried to index into something that is not a table. (For reference L-value.)");
                return NULL;
            }

            // Make sure the key is something reasonable to use as a key.
            dt = obR->getType();
            if(!obL->isValidKeyType(obR->getType())) {
                FLAG_ERROR(
                    derpSprintf(
                        "Tried to index into something with a %s. (For reference L-value.)",
                        derpObjectTypeToString(dt).c_str()));
                return NULL;
            }

            // Make sure the key is copyable in case we have to make an
            // entry for it in the table.
            if(!obR->getCopyable()) {
                FLAG_ERROR(
                    derpSprintf(
                        "Tried to index into something with a non-copyable index. (For reference L-value.)"));
                return NULL;
            }

            if(obL->getConst()) {
                FLAG_ERROR(
                    derpSprintf(
                        "Tried use a const table as a reference L-value."));
                return NULL;
            }

            DerpObject::Ref *result = obL->getInTablePtr(obR);

            if(!result) {
                DerpObject::Ref resultRef = vm->makeObject();
                resultRef->setInt(0);
                obL->setInTable(obR->copy(), resultRef);
                result = obL->getInTablePtr(obR);
                assert(result);
            }

            return result;

        } else {

            // Bad Lvalue error!
            FLAG_ERROR("Invalid L-value.");
            return NULL;

        }
    }

    DerpExecNode *DerpExecNode::copyTree(void) {

        DerpExecNode *ret = new DerpExecNode(
            vm, lineNumber, fileName);

        ret->type = type;

        for(unsigned int i = 0; i < children.size(); i++) {
            if(children[i]) {
                ret->children.push_back(children[i]->copyTree());
            } else {
                ret->children.push_back(NULL);
            }
        }

        if(data) {
            ret->data = data->copy();
        }

        return ret;
    }

    void DerpExecNode::markGCPass(unsigned int passNum) {

        if(data) {
            data->markGCPass(passNum);
        }

        for(unsigned int i = 0; i < children.size(); i++) {
            if(children[i]) {
                children[i]->markGCPass(passNum);
            }
        }
    }

    DerpExecNodeType DerpExecNode::getType(void) const {
        return type;
    }

    void DerpExecNode::setType(DerpExecNodeType type) {
        assert(type >= 0 && type < DERPEXEC_MAX);
        this->type = type;
    }

    unsigned int DerpExecNode::getLineNumber(void) const {
        return lineNumber;
    }

    std::string DerpExecNode::getFileName(void) const {
        return fileName;
    }

    void DerpExecNode::setData(DerpObject *data) {
        this->data = data;
    }

    DerpObject *DerpExecNode::getData(void) {
        return data;
    }

    PooledString::Ref DerpExecNode::getFileNameRef(void) const {
        return fileName;
    }

    bool derpExec_checkObjectType(
        DerpBasicType t, DerpObject::Ref r,
        DerpErrorState &errorState, DerpExecNode *node,
        DerpReturnType *returnType) {

        if(!r.getPtr()) {
            return false;
        }

        if(r->getType() != t) {

            errorState.setFileAndLine(
                node->getFileNameRef(),
                node->getLineNumber());

            errorState.addError(
                derpSprintf(
                    "Type error. Expected %s but got %s.",
                    derpObjectTypeToString(t).c_str(),
                    derpObjectTypeToString(r->getType()).c_str()));

            if(returnType) {
                *returnType = DERPRETURN_ERROR;
            }

            return false;
        }

        return true;
    }
}

