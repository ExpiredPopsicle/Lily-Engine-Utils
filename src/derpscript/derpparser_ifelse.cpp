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

    DerpExecNode *parseIfElse(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        unsigned int lineNumber = LINE_NUMBER();
        std::string fileName = derpSafeFileName(tokens, i);

        // Skip "if" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_IF)) {
            return NULL;
        }

        // Skip opening paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            return NULL;
        }

        // Parse the condition expression.
        DerpExecNode *condition = parseExpression(
            vm, tokens, i, errorState);

        if(!condition) {
            return NULL;
        }

        // Skip closing paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
            delete condition;
            return NULL;
        }

        // Parse the thing to do if the condition is true.
        DerpExecNode *ifAction = parseStatement(
            vm, tokens, i, errorState);

        if(!ifAction) {
            delete condition;
            return NULL;
        }

        // Check for an "else" statement.
        DerpExecNode *elseAction = NULL;
        if(TOKEN_TYPE() == DERPTOKEN_KEYWORD_ELSE) {

            // Skip "else"
            i++;

            // Parse else action.
            elseAction = parseStatement(
                vm, tokens, i, errorState);

            // If nothing was returned from that, then there was an error
            // parsing the "else" action.
            if(!elseAction) {
                delete condition;
                delete ifAction;
                return NULL;
            }
        }

        DerpExecNode *ret = new DerpExecNode(
            vm, lineNumber, fileName);
        ret->setType(DERPEXEC_IFELSE);
        ret->children.push_back(condition);
        ret->children.push_back(ifAction);
        ret->children.push_back(elseAction);

        return ret;

    }
}

