#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    DerpExecNode *parseVardec(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        // Skip "var" token.
        if(!EXPECT_AND_SKIP(DERPTOKEN_KEYWORD_VAR)) {
            return NULL;
        }

        if(TOKEN_TYPE(i) != DERPTOKEN_SYMBOL) {

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                derpSprintf("Expected variable name."));

            return NULL;
        }

        DerpExecNode *ret = new DerpExecNode(
            vm,
            derpSafeLineNumber(tokens, i),
            derpSafeFileName(tokens, i));

        ret->setType(DERPEXEC_VARIABLEDEC);
        DerpObject *data = new DerpObject(vm);
        data->setString(tokens[i]->str);
        ret->setData(data);

        // Skip symbol.
        i++;

        return ret;
    }
}

