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


