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

#pragma once

#include <string>
#include <vector>

namespace ExPop {

    class DerpErrorState;

    enum DerpTokenType {

        DERPTOKEN_OPENPAREN,
        DERPTOKEN_CLOSEPAREN,

        DERPTOKEN_OPENCURLY,
        DERPTOKEN_CLOSECURLY,
        DERPTOKEN_OPENBRACKET,
        DERPTOKEN_CLOSEBRACKET,
        DERPTOKEN_ENDSTATEMENT,

        // TODO: When everything else is in place, it might be a good idea
        // to move all the math ops into tokens instead of letting the
        // parser figure them out AFTER the lexer has already done it.
        DERPTOKEN_MATHOP,

        DERPTOKEN_COMMA,

        // ----------------------------------------------------------------------
        DERPTOKEN_START_LITERALS,
        // ----------------------------------------------------------------------

        DERPTOKEN_INT,
        DERPTOKEN_FLOAT,
        DERPTOKEN_STRING,

        // ----------------------------------------------------------------------
        DERPTOKEN_END_LITERALS,

        DERPTOKEN_KEYWORD_FUNCTION,
        DERPTOKEN_KEYWORD_VAR,
        DERPTOKEN_KEYWORD_BREAK,
        DERPTOKEN_KEYWORD_CONTINUE,
        DERPTOKEN_KEYWORD_RETURN,
        DERPTOKEN_KEYWORD_DEBUGOUT,
        DERPTOKEN_KEYWORD_IF,
        DERPTOKEN_KEYWORD_ELSE,
        DERPTOKEN_KEYWORD_WHILE,
        DERPTOKEN_KEYWORD_DO,
        DERPTOKEN_KEYWORD_FOR,

        DERPTOKEN_SYMBOL,

        DERPTOKEN_JUNK
    };

    class DerpToken {
    public:

        DerpToken(
            const std::string &inStr,
            DerpTokenType inType,
            const std::string &inFileName,
            unsigned int inLineNumber) {

            str = inStr;
            type = inType;
            lineNumber = inLineNumber;
            fileName = inFileName;
        }

        std::string str;
        DerpTokenType type;

        unsigned int lineNumber;
        std::string fileName;
    };

    /// Tokenize a string.
    bool derpGetTokens(
        const std::string &str,
        std::vector<DerpToken*> &outTokens,
        DerpErrorState &errorState,
        const std::string &fileName);

    std::string derpTokenTypeToString(DerpTokenType t);
}

