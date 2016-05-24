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

#pragma once

enum BlockType {
    BLOCKTYPE_NONE,

    BLOCKTYPE_TODO,
    BLOCKTYPE_FIXME,
    BLOCKTYPE_DONE,
};

const char extraDataReadOnlyTag[] = "+ Ext: ";
const char extraDataNotesHeader[] = "+ Saved Notes:";

struct CommentBlock {
    int startLineNumber;
    int startPositionInLine;
    string filename;
    string comment; // Read from source. Save to org.
    int issueId;
    BlockType type;

    bool needsNewId;

    vector<string> extraData; // Read from org. Save to org.
};

// This is so we can keep track of things that it's "safe" to mess
// with.
struct GitState {
    map<string, bool> modifiedFiles;
    map<string, bool> trackedFiles;
};


// Just get the string length for the initial block token.
unsigned int getStartLengthForBlockType(BlockType type);

BlockType getBlockType(const std::string &str);

int getIssueIdNumber(
    const std::string &str,
    unsigned int *startOfActualComment = NULL);

