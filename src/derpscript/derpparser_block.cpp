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

