#include "derpparser_internal.h"
using namespace std;

namespace ExPop {

    bool derpParser_expectAndIncrement(
        DerpTokenType type, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState) {

        // TODO: Once we have the lexer actually use a table of strings
        // and types, this should look up in that table for a string
        // representation of what we wanted.

        if(i >= tokens.size()) {

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                derpSprintf(
                    "Ran out of tokens when expecting %s.",
                    derpTokenTypeToString(type).c_str()));

            return false;
        }

        if(tokens[i]->type != type) {

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, i),
                derpSafeLineNumber(tokens, i));

            errorState.addError(
                derpSprintf(
                    "Expected a %s but found a %s.",
                    derpTokenTypeToString(type).c_str(),
                    derpTokenTypeToString(tokens[i]->type).c_str()));

            return false;
        }

        i++;

        return true;
    }

    DerpTokenType derpToken(
        const std::vector<DerpToken*> tokens,
        const unsigned int &i) {

        if(i >= tokens.size()) return DERPTOKEN_JUNK;
        return tokens[i]->type;
    }

    // ----------------------------------------------------------------------
    //  Wrapper for the parser and lexer.
    // ----------------------------------------------------------------------

    DerpExecNode *derpParseText(
        DerpVM *vm, const std::string &str,
        DerpErrorState &errorState,
        const std::string &fileName) {

        vector<DerpToken*> tokens;

        if(!derpGetTokens(str, tokens, errorState, fileName)) {
            // Failed at tokenization stage.
            return NULL;
        }

        // Now parse it.

        unsigned int position = 0;
        DerpExecNode *out = parseBlock(
            vm, tokens, position, errorState, true);

        for(unsigned int i = 0; i < tokens.size(); i++) {
            delete tokens[i];
        }

        if(!out) {
            // Failed to parse something.
            return NULL;
        }

        if(position != tokens.size()) {

            // If we're not at the end of input right now, something went
            // bad.

            errorState.setFileAndLineDirect(
                derpSafeFileName(tokens, position),
                derpSafeLineNumber(tokens, position));

            errorState.addError(
                "Expected end of input.");

            return NULL;
        }

        return out;
    }
}

