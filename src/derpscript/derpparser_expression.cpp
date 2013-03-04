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

#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    // ----------------------------------------------------------------------
    //  Operator classification and parsing.
    // ----------------------------------------------------------------------

    static DerpExecNode *parseOperator(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        DerpExecNode *ret = NULL;
        istringstream inStr(tokens[i]->str);

        if(TOKEN_TYPE(i) == DERPTOKEN_MATHOP) {
            ret = new DerpExecNode(
                vm,
                derpSafeLineNumber(tokens, i),
                derpSafeFileName(tokens, i));

            if(tokens[i]->str.size() == 1) {

                // Opcode is a single character.
                switch(tokens[i]->str[0]) {
                    case '+': ret->setType(DERPEXEC_ADD); break;
                    case '-': ret->setType(DERPEXEC_SUBTRACT); break;
                    case '*': ret->setType(DERPEXEC_MULTIPLY); break;
                    case '/': ret->setType(DERPEXEC_DIVIDE); break;
                    case '!': ret->setType(DERPEXEC_NOT); break;
                    case '=': ret->setType(DERPEXEC_ASSIGNMENT); break;
                    case '>': ret->setType(DERPEXEC_GT); break;
                    case '<': ret->setType(DERPEXEC_LT); break;
                    default:
                        // TODO: Implement the rest of the math ops.
                        assert(!"Unimplemented math op.");
                }

            } else if(tokens[i]->str.size() == 2) {

                if(tokens[i]->str == ":=") {
                    ret->setType(DERPEXEC_REFASSIGNMENT);
                } else if(tokens[i]->str == "||") {
                    ret->setType(DERPEXEC_OR);
                } else if(tokens[i]->str == "&&") {
                    ret->setType(DERPEXEC_AND);
                } else if(tokens[i]->str == "==") {
                    ret->setType(DERPEXEC_EQ);
                } else if(tokens[i]->str == "!=") {
                    ret->setType(DERPEXEC_NEQ);
                } else if(tokens[i]->str == "++") {
                    ret->setType(DERPEXEC_INCREMENT);
                } else if(tokens[i]->str == "--") {
                    ret->setType(DERPEXEC_DECREMENT);
                } else if(tokens[i]->str == ">=") {
                    ret->setType(DERPEXEC_GE);
                } else if(tokens[i]->str == "<=") {
                    ret->setType(DERPEXEC_LE);
                } else {
                    // TODO: All other operators. (Or normal error.)
                    assert(!"Unimplemented two-character math op.");
                }

            } else {
                assert(!"Bad number of characters for a math op.");
            }

        } else if(TOKEN_TYPE(i) == DERPTOKEN_OPENPAREN) {

            // Function call "operator".
            return parseFunctionCall(vm, tokens, i, errorState);

        } else if(TOKEN_TYPE(i) == DERPTOKEN_OPENBRACKET) {

            // Index "operator".
            return parseIndex(vm, tokens, i, errorState);
        }

        // Operator parse error.
        if(!ret) {
            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                derpSprintf("Invalid operator: \"%s\".", tokens[i]->str.c_str()));
        }

        i++;
        return ret;
    }

    static int getOpPrecedence(DerpExecNode *node) {

        // The return values just need to be relative to each other.
        // Feel free to mess with the ranges as long as the relative
        // values are consistent.

        switch(node->getType()) {

            case DERPEXEC_REFASSIGNMENT:
            case DERPEXEC_ASSIGNMENT:
                return 2;

            case DERPEXEC_OR:
            case DERPEXEC_AND:
                return 3;

            case DERPEXEC_EQ:
            case DERPEXEC_NEQ:
                return 4;

                // TODO: GE, LE
            case DERPEXEC_GE:
            case DERPEXEC_LE:
            case DERPEXEC_GT:
            case DERPEXEC_LT:
                return 10;

            case DERPEXEC_ADD:
            case DERPEXEC_SUBTRACT:
                return 5;

            case DERPEXEC_MULTIPLY:
            case DERPEXEC_DIVIDE:
                return 6;

            case DERPEXEC_NOT:
            case DERPEXEC_BINARYNOT:
                return 9;

            default:
                // TODO: Maybe make this less fatal.
                assert(!"Tried to get op precedence for something unimplemented!");
        }
    }

    static bool opIsUnaryPrefix(const DerpExecNode *node) {

        switch(node->getType()) {

            case DERPEXEC_NOT:
            case DERPEXEC_BINARYNOT:
                return true;

            default:
                return false;
        }
    }

    static bool opIsUnaryPostfix(const DerpExecNode *node) {

        switch(node->getType()) {

            case DERPEXEC_INCREMENT:
            case DERPEXEC_DECREMENT:
                return true;

            case DERPEXEC_FUNCTIONCALL:
                return true;

            case DERPEXEC_INDEX:
                return true;

            default:
                return false;
        }
    }

    static bool opIsUnary(const DerpExecNode *node) {
        return opIsUnaryPostfix(node) || opIsUnaryPrefix(node);
    }

    // ----------------------------------------------------------------------
    //  Shift/reduce expression parser
    // ----------------------------------------------------------------------

    static void reduce(
        std::vector<DerpExecNode*> &opStack,
        std::vector<DerpExecNode*> &valStack) {

        // TODO: Maybe make this less fatal.
        assert(opStack.size() >= 1);

        DerpExecNode *reducedNode = opStack[opStack.size() - 1];
        opStack.erase(opStack.end() - 1);

        if(opIsUnary(reducedNode)) {
            reducedNode->children.push_back(valStack[valStack.size() - 1]);
            valStack.erase(valStack.end() - 1);
        } else {
            reducedNode->children.push_back(valStack[valStack.size() - 2]);
            reducedNode->children.push_back(valStack[valStack.size() - 1]);
            valStack.erase(valStack.end() - 1);
            valStack.erase(valStack.end() - 1);
        }

        valStack.push_back(reducedNode);
    }

    static void reducePostfix(
        std::vector<DerpExecNode*> &valStack,
        DerpExecNode *postfixNode) {

        // TODO: Make less fatal.
        assert(valStack.size() >= 1);

        if(!postfixNode->children.size()) {
            postfixNode->children.push_back(NULL);
        } else {
            assert(!postfixNode->children[0]);
        }

        postfixNode->children[0] = valStack[valStack.size() - 1];
        valStack[valStack.size() - 1] = postfixNode;
    }

    // This is for cleaning up on errors.
    static void clearStack(vector<DerpExecNode*> &s) {
        for(unsigned int i = 0; i < s.size(); i++) {
            delete s[i];
        }
        s.clear();
    }

    enum ExpressionParserState {
        PARSERSTATE_VALUE_OR_PREFIX,
        PARSERSTATE_OPERATOR_OR_END
    };

    DerpExecNode *parseExpression(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        ExpressionParserState state = PARSERSTATE_VALUE_OR_PREFIX;

        bool reachedEnd = false;

        vector<DerpExecNode*> valueStack;
        vector<DerpExecNode*> operatorStack;

        while(!reachedEnd) {

            switch(state) {

                case PARSERSTATE_VALUE_OR_PREFIX: {

                    if(TOKEN_TYPE(i) == DERPTOKEN_OPENPAREN) {

                        // Parse sub-expression.

                        // Skip first parenthesis.
                        EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN);

                        DerpExecNode *newNode = parseExpression(
                            vm, tokens, i, errorState);

                        if(!newNode) {
                            clearStack(valueStack);
                            clearStack(operatorStack);
                            return NULL;
                        }

                        // The sub-parsing should have ended on a closed parenthesis.
                        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
                            clearStack(valueStack);
                            clearStack(operatorStack);
                            delete newNode;
                            return NULL;
                        }

                        valueStack.push_back(newNode);

                        state = PARSERSTATE_OPERATOR_OR_END;

                    } else {

                        // NOTE: derpParser_parseValue will fail in
                        // cases of prefix operators! This is okay.
                        // Just make sure the error message comes out
                        // here when it's a complete failure.
                        bool innerError = true;
                        DerpExecNode *newNode = derpParser_parseValue(
                            vm, tokens, i, errorState, innerError);

                        if(newNode) {

                            // Found a normal value.
                            valueStack.push_back(newNode);
                            state = PARSERSTATE_OPERATOR_OR_END;

                        } else {

                            if(!innerError) {
                                // Must be a prefix operator.
                                newNode = parseOperator(
                                    vm, tokens, i, errorState);
                            }

                            if(!innerError && newNode && opIsUnaryPrefix(newNode)) {

                                operatorStack.push_back(newNode);

                            } else {

                                clearStack(valueStack);
                                clearStack(operatorStack);

                                if(!newNode) {
                                    // parseOperator failed.
                                    return NULL;
                                }

                                errorState.setFileAndLineDirect(
                                    newNode->getFileName(),
                                    newNode->getLineNumber());

                                errorState.addError(
                                    "Expected value or prefix operator.");

                                delete newNode;
                                return NULL;
                            }
                        }
                    }

                } break;

                case PARSERSTATE_OPERATOR_OR_END: {

                    DerpTokenType tokenType = TOKEN_TYPE(i);

                    // We might have to expand this list if we decide
                    // that more things need to count as the end of an
                    // expression.
                    if(tokenType == DERPTOKEN_CLOSEPAREN ||
                       tokenType == DERPTOKEN_CLOSEBRACKET ||
                       tokenType == DERPTOKEN_ENDSTATEMENT ||
                       tokenType == DERPTOKEN_COMMA) {

                        // Found the end of this expression or
                        // sub-expression.
                        reachedEnd = true;

                    } else {

                        // TODO: We need a way to handle postfix
                        // operators. Especially for function calls
                        // (open paren after value) or array indexing
                        // (open bracket after value).

                        DerpExecNode *newNode = parseOperator(
                            vm, tokens, i, errorState);

                        if(newNode) {

                            if(opIsUnaryPostfix(newNode)) {

                                reducePostfix(valueStack, newNode);

                                // Parser state stays the same.

                            } else {

                                // Reduce until we see something of
                                // lower operator precedence on the
                                // top of the operator stack.
                                while(
                                    operatorStack.size() &&
                                    getOpPrecedence(operatorStack[operatorStack.size() - 1]) > getOpPrecedence(newNode)) {

                                    reduce(operatorStack, valueStack);
                                }

                                operatorStack.push_back(newNode);
                                state = PARSERSTATE_VALUE_OR_PREFIX;
                            }

                        } else {

                            // parseOperator failed.

                            // Error message was generated in
                            // parseOperator or below. Just clean up
                            // and return.
                            clearStack(valueStack);
                            clearStack(operatorStack);
                            return NULL;
                        }
                    }

                } break;

            }

            if(!TOKENS_LEFT()) {

                errorState.setFileAndLineDirect(
                    derpSafeFileName(tokens, i),
                    derpSafeLineNumber(tokens, i));

                errorState.addError(
                    "Ran out of tokens in the middle of an exprssion.");

                reachedEnd = true;
            }
        }

        // Collapse all remaining operators.
        while(operatorStack.size()) {
            reduce(operatorStack, valueStack);
        }

        // TODO: Make these less fatal?
        assert(valueStack.size() == 1);
        assert(operatorStack.size() == 0);

        return valueStack[0];
    }
}

