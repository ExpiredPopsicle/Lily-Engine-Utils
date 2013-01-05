#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    // Name is slight misnomer. Actually only parses starting at the
    // open parenthesis.
    DerpExecNode *parseIndex(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        DerpExecNode *ret = new DerpExecNode(
            vm, LINE_NUMBER(),
            derpSafeFileName(tokens, i));

        ret->setType(DERPEXEC_INDEX);

        if(!EXPECT_AND_SKIP(DERPTOKEN_OPENBRACKET)) {
            delete ret;
            return NULL;
        }

        if(!TOKENS_LEFT()) {
            delete ret;

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                "Ran out of tokens while parsing index operator.");

            return NULL;
        }

        // Make a slot to put the table/array evaluation into. (This will
        // get filled in as normal postfix handling.)
        ret->children.push_back(NULL);

        DerpExecNode *child = parseExpression(
            vm, tokens, i, errorState);

        if(!child) {
            delete ret;
            return NULL;
        }

        ret->children.push_back(child);

        if(!EXPECT_AND_SKIP(DERPTOKEN_CLOSEBRACKET)) {
            delete ret;
            return NULL;
        }

        return ret;
    }
}

