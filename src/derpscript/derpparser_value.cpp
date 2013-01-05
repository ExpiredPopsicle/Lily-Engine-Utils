#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *derpParser_parseValue(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState,
        bool &innerError) {

        if(!TOKENS_LEFT()) return NULL;

        DerpExecNode *ret = NULL;
        istringstream inStr(tokens[i]->str);
        innerError = false;

        DerpObject *literalData = new DerpObject(vm);

        switch(TOKEN_TYPE()) {

            case DERPTOKEN_INT: {

                // Literal integer.
                int intVal;
                inStr >> intVal;
                literalData->setInt(intVal);

            } break;

            case DERPTOKEN_FLOAT: {

                // Literal float.
                float floatVal;
                inStr >> floatVal;
                literalData->setFloat(floatVal);

            } break;

            case DERPTOKEN_STRING: {

                // Literal string.
                literalData->setString(tokens[i]->str);

            } break;

            case DERPTOKEN_SYMBOL: {

                // Variable name.
                ret = new DerpExecNode(
                    vm,
                    derpSafeLineNumber(tokens, i),
                    derpSafeFileName(tokens, i));

                ret->setType(DERPEXEC_VARLOOKUP);
                literalData->setString(tokens[i]->str);
                literalData->setConst(true);
                ret->setData(literalData);
                i++;

            } break;

            case DERPTOKEN_KEYWORD_FUNCTION: {

                ret = parseFunction(vm, tokens, i, errorState);
                innerError = true;

            } break;

            case DERPTOKEN_KEYWORD_VAR: {

                ret = parseVardec(vm, tokens, i, errorState);
                innerError = true;

            } break;

            default:
                // Failed to parse as value. Might just be a prefix
                // operator, though. Calling function should handle those
                // errors.
                return NULL;
                break;
        }

        if(!ret) {
            ret = new DerpExecNode(
                vm,
                derpSafeLineNumber(tokens, i),
                derpSafeFileName(tokens, i));

            ret->setType(DERPEXEC_LITERAL);
            ret->setData(literalData);
            literalData->setConst(true);
            i++;
        }

        return ret;
    }
}

