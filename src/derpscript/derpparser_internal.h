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

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

#include "derperror.h"
#include "derpobject.h"
#include "derpparser.h"
#include "derpexecnode.h"
#include "derplexer.h"
#include "derpvm.h"
#include "pooledstring.h"

namespace ExPop {

    // ----------------------------------------------------------------------
    //  Some #defines for things that keep popping up
    // ----------------------------------------------------------------------

  #define TOKENS_LEFT() (i < tokens.size())

    bool derpParser_expectAndIncrement(
        DerpTokenType type, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

  #define EXPECT_AND_SKIP(x) derpParser_expectAndIncrement(x, tokens, i, errorState)

    DerpTokenType derpToken(
        const std::vector<DerpToken*> tokens,
        const unsigned int &i);

    // TOKEN_TYPE includes a safety check like TOKENS_LEFT. But don't use
    // it for while loop conditions without TOKENS_LEFT or it could run
    // forever.
  #define TOKEN_TYPE(x) derpToken(tokens, i)

  #define LINE_NUMBER() derpSafeLineNumber(tokens, i)
  #define FILE_NAME() derpSafeFileName(tokens, i)

    // ----------------------------------------------------------------------
    //  Internal parser stuff
    // ----------------------------------------------------------------------

    // TODO: Make a DerpParserState to handle the identical parameters to
    // all of these functions.

    DerpExecNode *derpParser_parseValue(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState,
        bool &innerError);

    DerpExecNode *parseFunction(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseVardec(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    /// Start parsing a function call. i must be the index of the starting
    /// parenthesis!
    DerpExecNode *parseFunctionCall(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseExpression(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseStatement(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseFunction(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseVardec(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseIfElse(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseWhileLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseDoWhileLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseForLoop(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    DerpExecNode *parseBlock(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState,
        bool ignoreBraces);

    DerpExecNode *parseIndex(
        DerpVM *vm, std::vector<DerpToken*> &tokens,
        unsigned int &i, DerpErrorState &errorState);

    // ----------------------------------------------------------------------
    //  Debug junk
    // ----------------------------------------------------------------------

    std::string getStringForNodeType(DerpExecNodeType type);
}
