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

