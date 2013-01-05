#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *parseReturn(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        DerpExecNode *ret = new DerpExecNode(
            vm,
            derpSafeLineNumber(tokens, i),
            derpSafeFileName(tokens, i));

        ret->setType(DERPEXEC_RETURN);

        // Skip "return".
        EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_RETURN);

        DerpExecNode *retExpression = parseExpression(
            vm, tokens, i, errorState);

        if(!retExpression) {
            delete ret;
            return NULL;
        }

        // Skip ';'
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete retExpression;
            delete ret;
            return NULL;
        }

        ret->children.push_back(retExpression);

        return ret;
    }

    DerpExecNode *parseDbgOut(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        DerpExecNode *ret = new DerpExecNode(
            vm,
            derpSafeLineNumber(tokens, i),
            derpSafeFileName(tokens, i));

        ret->setType(DERPEXEC_DEBUGPRINT);

        // Skip "dbgout".
        EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_DEBUGOUT);

        DerpExecNode *retExpression = parseExpression(
            vm, tokens, i, errorState);

        if(!retExpression) {
            delete ret;
            return NULL;
        }

        // Skip ';'
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete retExpression;
            delete ret;
            return NULL;
        }

        ret->children.push_back(retExpression);

        return ret;
    }

    DerpExecNode *parseStatement(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        // If statements, etc. Fall back on expression if no keywords are
        // seen.

        if(!TOKENS_LEFT()) {

            errorState.setFileAndLineDirect(
                "",
                0);

            errorState.addError(
                "No tokens to parse!");

            return NULL;
        }

        DerpTokenType tokenType = TOKEN_TYPE(i);

        switch(tokenType) {

            case DERPTOKEN_OPENCURLY:
                // Block.
                return parseBlock(vm, tokens, i, errorState, false);

            case DERPTOKEN_KEYWORD_IF:
                // If/else statement.
                return parseIfElse(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_WHILE:
                // While loop.
                return parseWhileLoop(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_DO:
                // Do/while loop.
                return parseDoWhileLoop(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_FOR:
                // Do/while loop.
                return parseForLoop(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_RETURN:
                // Return statement.
                return parseReturn(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_DEBUGOUT:
                // Debug output. (TODO: Remove this?)
                return parseDbgOut(vm, tokens, i, errorState);

            case DERPTOKEN_KEYWORD_BREAK:
            case DERPTOKEN_KEYWORD_CONTINUE:
                // This should cover any case where the keyword is
                // alone. Like break or continue statements.
            {
                DerpExecNode *ret = new DerpExecNode(
                    vm,
                    derpSafeLineNumber(tokens, i),
                    derpSafeFileName(tokens, i));

                switch(tokenType) {

                    case DERPTOKEN_KEYWORD_BREAK: ret->setType(DERPEXEC_BREAK); break;
                    case DERPTOKEN_KEYWORD_CONTINUE: ret->setType(DERPEXEC_CONTINUE); break;

                    default:
                        // Uh... forgot to handle something obvious?
                        assert(0);
                        break;
                }

                // Skip the keyword.
                EXPECT_AND_SKIP(tokenType);

                // Skip ';'
                if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
                    delete ret;
                    return NULL;
                }

                return ret;
            }

            default:
                break;
        }



        // Handle expressions.
        DerpExecNode *expressionNode = parseExpression(
            vm, tokens, i, errorState);

        if(!expressionNode) {
            // Expression failed to parse.
            return NULL;
        }

        // Make sure this ends with a semicolon or something.
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete expressionNode;
            return NULL;
        }

        return expressionNode;

    }
}

