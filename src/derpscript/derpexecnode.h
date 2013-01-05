#pragma once

#include <string>
#include <vector>

#include "derpobject.h"
#include "derperror.h"
#include "derperror_internal.h"
#include "pooledstring.h"

namespace ExPop {

    // This should probably all be considered internal.

    enum DerpExecNodeType {

        DERPEXEC_ERROROP,

        // Simple math operations.
        DERPEXEC_ADD,
        DERPEXEC_SUBTRACT,
        DERPEXEC_MULTIPLY,
        DERPEXEC_DIVIDE,
        DERPEXEC_ASSIGNMENT,
        DERPEXEC_REFASSIGNMENT,

        DERPEXEC_GT,
        DERPEXEC_LT,
        DERPEXEC_GE,
        DERPEXEC_LE,
        DERPEXEC_EQ,
        DERPEXEC_NEQ,
        DERPEXEC_INCREMENT,
        DERPEXEC_DECREMENT,

        // Unary math operations. (Prefix and postfix)
        DERPEXEC_BINARYNOT,

        // Unary math operations. (Prefix)
        DERPEXEC_NOT,

        // Literal data representation. Quoted strings, numbers, etc.
        DERPEXEC_LITERAL,

        // Variable lookup.
        DERPEXEC_VARLOOKUP,

        // Block of statements. Returns the result of the last thing
        // it evals. (Like LISP.)
        DERPEXEC_BLOCK,
        DERPEXEC_FREEBLOCK, // No curly braces or context switching.

        // First child evaluates to the function itself. Everything
        // else goes to parameters. Whole thing evals to whatever the
        // function returns.
        DERPEXEC_FUNCTIONCALL,

        DERPEXEC_RETURN,

        // Variable declaration. Modifies context.
        DERPEXEC_VARIABLEDEC,

        DERPEXEC_IFELSE,

        DERPEXEC_LOOP,

        DERPEXEC_BREAK,
        DERPEXEC_CONTINUE,

        DERPEXEC_DEBUGPRINT,

        DERPEXEC_INDEX,

        DERPEXEC_MAX
    };

    /// This type indicates all the ways some eval() call can return.
    /// break statements need to get passed up to the loop they break
    /// out of. return statements need to get passed up to the
    /// function they return from. Etc.
    enum DerpReturnType {
        DERPRETURN_NORMAL,
        DERPRETURN_FUNCTIONRETURN,
        DERPRETURN_BREAK,
        DERPRETURN_CONTINUE,
        DERPRETURN_ERROR,
    };

    class DerpContext;
    class DerpVM;
    class DerpErrorState;

    class DerpExecNode {
    public:

        DerpExecNode(
            DerpVM *vm,
            unsigned int lineNumber,
            const std::string &fileName);

        ~DerpExecNode(void);

        /// Execute this node hierarchy.
        DerpObject::Ref eval(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        /// Evaluate to a pointer to the object. This way the pointer
        /// can be reassigned. Note: Given the number of moving parts
        /// in the VM, the pointer is only valid immediately after
        /// this call! Only works for valid Lvalues. (Variable lookup,
        /// array indexing, etc.)
        DerpObject::Ref *evalPtr(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        /// Copy the entire hierarchy of nodes.
        DerpExecNode *copyTree(void);

        /// Get the type.
        DerpExecNodeType getType(void) const;

        /// Set the type. Note that until there are the appropriate
        /// children or data, this DerpExecNode might be in a bad
        /// state.
        void setType(DerpExecNodeType type);

        /// Get the line number for the token this was built from.
        unsigned int getLineNumber(void) const;

        /// Get the file name for the token this was built from.
        std::string getFileName(void) const;

        /// Get the file name as a reference.
        PooledString::Ref getFileNameRef(void) const;

        /// The order and number of child exec nodes depends on the
        /// type of the DerpExecNode.
        std::vector<DerpExecNode *> children;

        /// Set literal data, variable names, or other stuff.
        void setData(DerpObject *data);

        /// Get the literal data.
        DerpObject *getData(void);

    private:

        // For literal data stuff.
        DerpObject *data;

        unsigned int lineNumber;
        DerpExecNodeType type;

        void markGCPass(unsigned int passNum);

        // Helper functions so the main eval() doesn't get too
        // bloated.

        DerpObject::Ref eval_functionCall(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        DerpObject::Ref eval_binaryMathOp(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        DerpObject::Ref eval_loop(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        DerpObject::Ref eval_unaryMathOp(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        DerpObject::Ref eval_index(
            DerpContext *context,
            DerpReturnType *returnType,
            DerpErrorState &errorState,
            void *userData);

        DerpVM *vm;
        PooledString::Ref fileName;

        friend class DerpObject;
    };
}

