#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *parseWhileLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        unsigned int lineNumber = LINE_NUMBER();
        std::string fileName = derpSafeFileName(tokens, i);

        // Skip "while" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_WHILE)) {
            return NULL;
        }

        // Skip open parenthesis.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            return NULL;
        }

        // Parse the condition expression.
        DerpExecNode *condition = parseExpression(
            vm, tokens, i, errorState);

        if(!condition) {
            return NULL;
        }

        // Skip closing parenthesis.
        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
            delete condition;
            return NULL;
        }

        // Parse the thing to do as long as the condition is true.
        DerpExecNode *whileAction = parseStatement(
            vm, tokens, i, errorState);

        if(!whileAction) {
            delete condition;
            return NULL;
        }

        DerpExecNode *ret = new DerpExecNode(
            vm, lineNumber, fileName);
        ret->setType(DERPEXEC_LOOP);
        ret->children.push_back(whileAction);
        ret->children.push_back(condition);
        ret->children.push_back(NULL);
        ret->children.push_back(NULL);
        ret->children.push_back(NULL);

        return ret;

    }

    DerpExecNode *parseDoWhileLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        unsigned int lineNumber = LINE_NUMBER();
        std::string fileName = derpSafeFileName(tokens, i);

        // Skip "do" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_DO)) {
            return NULL;
        }

        // Parse the thing to do as long as the condition is true.
        DerpExecNode *loopAction = parseStatement(
            vm, tokens, i, errorState);

        if(!loopAction) {
            return NULL;
        }

        // Skip "while" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_WHILE)) {
            delete loopAction;
            return NULL;
        }

        // Skip open paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            delete loopAction;
            return NULL;
        }

        // Parse the condition expression.
        DerpExecNode *condition = parseExpression(
            vm, tokens, i, errorState);

        if(!condition) {
            return NULL;
        }

        // Skip open paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
            delete loopAction;
            delete condition;
            return NULL;
        }

        // Skip ';'.
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete loopAction;
            delete condition;
            return NULL;
        }

        DerpExecNode *ret = new DerpExecNode(
            vm, lineNumber, fileName);
        ret->setType(DERPEXEC_LOOP);
        ret->children.push_back(loopAction);
        ret->children.push_back(NULL);
        ret->children.push_back(condition);
        ret->children.push_back(NULL);
        ret->children.push_back(NULL);

        return ret;
    }

    DerpExecNode *parseForLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        unsigned int lineNumber = LINE_NUMBER();
        std::string fileName = derpSafeFileName(tokens, i);

        // Skip "for" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_FOR)) {
            return NULL;
        }

        // Skip open paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            return NULL;
        }

        // Parse the init expression.
        DerpExecNode *initExpression = NULL;
        if(TOKEN_TYPE() != DERPTOKEN_ENDSTATEMENT) {

            initExpression = parseExpression(
                vm, tokens, i, errorState);

            if(!initExpression) {
                return NULL;
            }
        }

        // Skip ';'.
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete initExpression;
            return NULL;
        }

        // Parse the condition expression.
        DerpExecNode *conditionExpression = NULL;
        if(TOKEN_TYPE() != DERPTOKEN_ENDSTATEMENT) {

            conditionExpression = parseExpression(
                vm, tokens, i, errorState);

            if(!conditionExpression) {
                delete initExpression;
                return NULL;
            }
        }

        // Skip ';'.
        if(!EXPECT_AND_SKIP(DERPTOKEN_ENDSTATEMENT)) {
            delete conditionExpression;
            delete initExpression;
            return NULL;
        }

        // Parse the condition expression.
        DerpExecNode *iterateExpression = NULL;
        if(TOKEN_TYPE() != DERPTOKEN_CLOSEPAREN) {

            iterateExpression = parseExpression(
                vm, tokens, i, errorState);

            if(!conditionExpression) {
                delete conditionExpression;
                delete initExpression;
                return NULL;
            }
        }

        // Skip close paren.
        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN)) {
            delete iterateExpression;
            delete conditionExpression;
            delete initExpression;
            return NULL;
        }

        DerpExecNode *loopAction = parseStatement(
            vm, tokens, i, errorState);

        if(!loopAction) {
            delete iterateExpression;
            delete conditionExpression;
            delete initExpression;
            return NULL;
        }

        DerpExecNode *ret = new DerpExecNode(
            vm, lineNumber, fileName);
        ret->setType(DERPEXEC_LOOP);
        ret->children.push_back(loopAction);
        ret->children.push_back(NULL);
        ret->children.push_back(conditionExpression);
        ret->children.push_back(initExpression);
        ret->children.push_back(iterateExpression);

        return ret;

    }
}
