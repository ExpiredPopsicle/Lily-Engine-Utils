#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    // Name is slight misnomer. Actually only parses starting at the open
    // parenthesis.
    DerpExecNode *parseFunctionCall(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        DerpExecNode *ret = new DerpExecNode(
            vm, LINE_NUMBER(),
            derpSafeFileName(tokens, i));

        ret->setType(DERPEXEC_FUNCTIONCALL);

        // Skip opening parenthesis.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            delete ret;
            return NULL;
        }

        if(!TOKENS_LEFT()) {
            delete ret;

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                "Ran out of tokens while parsing function call.");

            return NULL;
        }

        // Let's make a slot to stick the function evaluation itself into.
        // (This will get filled in as normal postfix handling.)
        ret->children.push_back(NULL);

        while(TOKENS_LEFT()) {

            if(TOKEN_TYPE() == DERPTOKEN_CLOSEPAREN) {
                // This will only get hit if there are no parameters at
                // all.
                break;
            }

            DerpExecNode *child = parseExpression(
                vm, tokens, i, errorState);

            if(!child) {
                // Error message already handled in parseExpression.
                delete ret;
                return NULL;
            }

            ret->children.push_back(child);

            // Skip over commas.
            if(TOKENS_LEFT()) {

                if(TOKEN_TYPE() == DERPTOKEN_COMMA) {
                    i++; // Skip commas.
                } else if(TOKEN_TYPE() == DERPTOKEN_CLOSEPAREN) {
                    break; // Done parsing parameters.
                } else {

                    errorState.setFileAndLineDirect(
                        derpSafeFileName(tokens, i),
                        derpSafeLineNumber(tokens, i));

                    errorState.addError(
                        "Expected ',' or ')' while parsing function call.");

                    delete ret;
                    return NULL;
                }
            } else {

                errorState.setFileAndLineDirect(
                    derpSafeFileName(tokens, i),
                    derpSafeLineNumber(tokens, i));

                errorState.addError(
                    "Ran out of tokens while parsing function call.");

                delete ret;
                return NULL;
            }
        }

        // Skip end paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
            delete ret;
            return NULL;
        }

        return ret;
    }
}


