// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
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

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
using namespace std;

#include <lilyengine/malstring.h>
using namespace ExPop;

#include "common.h"

unsigned int getStartLengthForBlockType(BlockType type) {
    switch(type) {
        case BLOCKTYPE_TODO:
        case BLOCKTYPE_DONE:
            return 4;
        case BLOCKTYPE_FIXME:
            return 5;
        default:
            return 0;
    }
}

BlockType getBlockType(const std::string &str) {
    if(strStartsWith("TODO", str)) {
        return BLOCKTYPE_TODO;
    } else if(strStartsWith("DONE", str)) {
        return BLOCKTYPE_DONE;
    } else if(strStartsWith("FIXME", str)) {
        return BLOCKTYPE_FIXME;
    }
    return BLOCKTYPE_NONE;
}

int getIssueIdNumber(
    const std::string &str,
    unsigned int *startOfActualComment) {

    int ret = -1;
    BlockType blockType = getBlockType(str);
    unsigned int startLength = getStartLengthForBlockType(blockType);
    // unsigned int startPos = startLength > 0 ? (startLength + 1) : startLength;
    unsigned int startPos = startLength;
    unsigned int endPos = startPos;

    if(blockType > BLOCKTYPE_NONE) {

        // Skip a space at the start of the tag if it's there.
        // (Even though it shouldn't be.)
        while(
            startLength < str.size() &&
            isWhiteSpace(str[startLength])) {

            startLength++;
            endPos++;
            startPos++;
        }

        if(startLength < str.size()) {

            // Pull out an issue id number.
            if(str[startLength] == '[') {

                // Skip the '['.
                startPos++;
                endPos++;

                // Find the end of the number.
                while(
                    endPos < str.size() &&
                    ((str[endPos] >= '0' &&
                      str[endPos] <= '9') || str[endPos] == '-')) {

                    endPos++;
                }

                // Get the number substring and convert it to an
                // unsigned int.
                string issueId = str.substr(startPos, endPos - startPos);
                istringstream issueIdStr(issueId);
                issueIdStr >> ret;
            }

            // Skip the ending bracket if it's there.
            if(endPos < str.size() && str[endPos] == ']')
                endPos++;
        }
    }

    // Record the start of the actual comment.
    if(startOfActualComment) {

        // Skip any remaining whitespace or :.
        while(endPos < str.size() && (isWhiteSpace(str[endPos]) || str[endPos] == ':')) {
            endPos++;
        }

        *startOfActualComment = endPos;
    }

    return ret;
}


