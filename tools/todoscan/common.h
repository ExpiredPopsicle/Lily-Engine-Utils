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

