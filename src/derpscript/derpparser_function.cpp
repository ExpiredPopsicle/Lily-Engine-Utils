#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *parseFunction(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        // Function keyword.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_FUNCTION)) {
            return NULL;
        }

        // Open parenthesis.
        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENPAREN)) {
            return NULL;
        }

        // Parse parameters.
        vector<string> *paramsList = new vector<string>();

        // FIXME: The way this loop is set up right now, it'll allow a
        // leading comma after the last parameter. "function(a, b,)" would
        // work, but this should be a syntax error.

        while(TOKENS_LEFT() && TOKEN_TYPE() != DERPTOKEN_CLOSEPAREN) {

            if(TOKEN_TYPE() != DERPTOKEN_SYMBOL) {

                errorState.setFileAndLineDirect(
                    derpSafeFileName(tokens, i),
                    derpSafeLineNumber(tokens, i));

                errorState.addError(
                    derpSprintf(
                        "Bad function parameter name: %s",
                        TOKENS_LEFT() ? tokens[i]->str.c_str() : "<EOF>"));

                delete paramsList;
                return NULL;
            }

            // Get param name.
            string paramName = tokens[i]->str;
            i++;

            // Do some error checking before we bother adding it to the list.
            if(TOKENS_LEFT() && TOKEN_TYPE() == DERPTOKEN_CLOSEPAREN) {

                // Don't skip the next token yet so we hit the end
                // correctly.

            } else {

                if(!EXPECT_AND_SKIP(DERPTOKEN_COMMA)) {
                    delete paramsList;
                    return NULL;
                }
            }

            paramsList->push_back(paramName);
        }

        // Skip end parenthesis.
        EXPECT_AND_SKIP(DERPTOKEN_CLOSEPAREN);

        unsigned int startLineNumber = LINE_NUMBER();
        std::string startFileName = derpSafeFileName(tokens, i);

        // Parse statement.
        DerpExecNode *functionExecNode = parseStatement(
            vm, tokens, i, errorState);

        if(!functionExecNode) {
            // Parse error.
            delete paramsList;
            return NULL;
        }

        // Possibly FIXME: We have the option of either wrapping this up
        // into a literal value (function as a data object the same way
        // we'd specify a literal number or string) or making an execnode
        // that returns a function object of one of its children
        // execnodes. I'm going to go with the literal value approach for
        // now.

        DerpObject *funcObject = new DerpObject(vm);
        funcObject->setFunction(functionExecNode, paramsList);
        funcObject->setConst(true);

        DerpExecNode *ret = new DerpExecNode(
            vm,
            startLineNumber,
            startFileName);

        ret->setType(DERPEXEC_LITERAL);
        ret->setData(funcObject);

        return ret;
    }
}

