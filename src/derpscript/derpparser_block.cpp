#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *parseBlock(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState,
        bool ignoreBraces) {

        if(!ignoreBraces) {
            // Skip opening curly brace.
            if(!EXPECT_AND_SKIP(DERPTOKEN_OPENCURLY)) {
                return NULL;
            }
        }

        DerpExecNode *ret = new DerpExecNode(
            vm,
            derpSafeLineNumber(tokens, i),
            derpSafeFileName(tokens, i));

        ret->setType(ignoreBraces ? DERPEXEC_FREEBLOCK : DERPEXEC_BLOCK);

        while(
            TOKENS_LEFT() &&
            (ignoreBraces || TOKEN_TYPE() != DERPTOKEN_CLOSECURLY)) {

            DerpExecNode *subNode = parseStatement(
                vm, tokens, i, errorState);

            if(!subNode) {
                // Error happened in recursion.
                delete ret;
                return NULL;
            }

            ret->children.push_back(subNode);
        }

        if(!ignoreBraces) {
            // Skip closing curly brace.
            if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSECURLY)) {
                delete ret;
                return NULL;
            }
        }

        return ret;

    }
}

