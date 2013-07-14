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
#include <vector>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

// This is all testing junk...
// ----------------------------------------------------------------------

// TODO: Herp some derps.

// TODO: More complicated blocks, sometimes spanning multiple lines!
//   (This is not actually a TODO. I'm actually testing that feature
//   with this very comment block. Derp.)
//
//   Paragraph test.

// TODO[12345]: This is a test of an issue with a number.

// TODO [12346]: This is a proper test of an issue with a number.

// TODO         [12347]: Here's a really bad one.

// TODO[]: Degenerate test1.

// TODO[: Degenerate test2.

// TODO [100]: Herp derp derp
//
// This is some source file extra data.

string testThinger  = "// TODO: Don't read this string";
string testThinger2 = "\" // TODO: Don't read this string either";

// ----------------------------------------------------------------------

// Now serious issues...

// FIXME: "DONE" stuff keeps ending up at the top of whatever source
// file they were supposedly originally in.
//
// That is, stuff from the org-mode file that isn't in the source
// file, which we assumed to be done.

// FIXME: (Maybe) Stuff is showing up as DONE in the org file, but the
// issue still exists in the cpp file.
//
// This is because the ones in the org file had the issue number
// assigned to them while the stuff in the cpp file was unchanged.
// Presumed that this bug will go away once we actually start writing
// files.

// TODO: Add file writing.

// TODO: Determine and implement correct behavior for unrelated lines
// in org files.

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

// Just get the string length for the initial block token.
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
    unsigned int *startOfActualComment = NULL) {

    int ret = -1;
    BlockType blockType = getBlockType(str);
    unsigned int startLength = getStartLengthForBlockType(blockType);
    unsigned int startPos = startLength > 0 ? (startLength + 1) : startLength;
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

// Load a file into a bunch of lines.
void loadFileLines(
    const std::string &filename,
    vector<string> &lines) {

    int fileLen = 0;
    char *fileData = FileSystem::loadFile(filename, &fileLen, true);

    if(fileData) {

        // Separate file into lines.
        stringTokenize(fileData, "\n", lines, true);

        delete[] fileData;

    } else {

        cerr << "Error loading " << filename << " or empty file." << endl;
        exit(1);
    }
}

void setBlockTypesByTag(vector<CommentBlock*> &commentBlocks) {

    // Now go through and set block types for everything based on the
    // tag at the start of the block.
    for(unsigned int i = 0; i < commentBlocks.size(); i++) {

        BlockType blockType = BLOCKTYPE_NONE;
        unsigned int startLength = 0;

        blockType = getBlockType(commentBlocks[i]->comment);

        startLength = getStartLengthForBlockType(blockType);
        commentBlocks[i]->type = blockType;
        commentBlocks[i]->issueId = getIssueIdNumber(commentBlocks[i]->comment);

        if(commentBlocks[i]->issueId != -1) {
            commentBlocks[i]->needsNewId = false;
        }
    }

}

// This function just creates CommentBlocks from a source file. It can
// leave them partially incomplete (lacking issue numbers if they
// didn't have them already).
void processSourceFile(
    const std::string &filename,
    vector<string> &lines,
    vector<CommentBlock*> &commentBlocks) {

    ostringstream currentBlockOut;
    int blockStartLine = -1;
    int blockStartPosInLine = -1;
    bool lastCommentLineWasNewline = false;

    for(unsigned int lineNum = 0; lineNum < lines.size(); lineNum++) {

        // Find the start of a C++ comment
        unsigned int charNum = 0;
        while(charNum < lines[lineNum].size()) {

            if(lines[lineNum][charNum] == '"' || lines[lineNum][charNum] == '\'') {

                // Found the start of a quoted string. It's not
                // okay to try to parse comments in here, so read
                // until the end of the string.

                char quoteStartChar = lines[lineNum][charNum];
                charNum++; // Skip past the starting quote.

                bool skipNext = false;
                while(charNum < lines[lineNum].size()) {
                    if(!skipNext) {
                        if(lines[lineNum][charNum] == '\\') {
                            skipNext = true;
                        }
                        if(lines[lineNum][charNum] == quoteStartChar) {
                            break;
                        }
                    } else {
                        skipNext = false;
                    }
                    charNum++;
                }
            }

            if(charNum + 1 < lines[lineNum].size() &&
               lines[lineNum][charNum] == '/' &&
               lines[lineNum][charNum+1] == '/'
                ) {
                break;
            }

            charNum++;

        }

        if(charNum < lines[lineNum].size()) {

            // If we aren't at the end of a line, I think that
            // means we've found a comment.

            // cout << charNum << " " << lines[lineNum].size() << endl;
            // cout << "Found a comment on line " << lineNum << endl;

            // Skip the comment marker.
            charNum += 2;

            if(blockStartPosInLine == -1) {

                // Find the actual start of the line after all the
                // whitespace.
                blockStartPosInLine = charNum;
                while(blockStartPosInLine < lines[lineNum].size() && isWhiteSpace(lines[lineNum][blockStartPosInLine])) {
                    blockStartPosInLine++;
                }
            }

            // // Skip past any whitespace.
            // while(charNum < lines[lineNum].size() && isWhiteSpace(lines[lineNum][charNum])) {
            //     charNum++;
            // }

            string commentStr = strTrim(lines[lineNum].substr(charNum));

            bool addWhiteSpaceToLineStart = !lastCommentLineWasNewline;

            if(!commentStr.size()) {
                // Blank line comment = newline divider thingy.
                commentStr = "\n";
                lastCommentLineWasNewline = true;
                // cout << "NEWLINE" << endl;
            } else {
                // cout << "NOT NEWLINE: " << commentStr << endl;
                lastCommentLineWasNewline = false;
            }

            if(blockStartLine == -1) {
                blockStartLine = lineNum;
                currentBlockOut << commentStr;
            } else {
                currentBlockOut << (addWhiteSpaceToLineStart ? " " : "") << commentStr;
            }

        } else {

            if(blockStartLine != -1) {

                // We found the first line that isn't a comment after
                // a comment block. Finish up the block and add it to
                // the list.

                CommentBlock *newBlock = new CommentBlock();
                newBlock->startLineNumber = blockStartLine;
                newBlock->startPositionInLine = blockStartPosInLine;
                newBlock->issueId = -1;
                newBlock->comment = currentBlockOut.str();
                newBlock->filename = filename;
                newBlock->type = BLOCKTYPE_NONE;
                newBlock->needsNewId = true;
                commentBlocks.push_back(newBlock);

                blockStartLine = -1;
                blockStartPosInLine = -1;
                currentBlockOut.str("");
            }
        }

    }

    // Finish off the last comment block if we ended on a comment.
    if(blockStartLine != -1) {

        CommentBlock *newBlock = new CommentBlock();
        newBlock->startLineNumber = blockStartLine;
        newBlock->startPositionInLine = blockStartPosInLine;
        newBlock->issueId = -1;
        newBlock->comment = currentBlockOut.str();
        newBlock->filename = filename;
        newBlock->type = BLOCKTYPE_NONE;
        newBlock->needsNewId = true;
        commentBlocks.push_back(newBlock);

        blockStartLine = -1;
        currentBlockOut.str("");
    }

}

// Fix the comment lines themselves in each file. (Or our buffers of
// those files, anyway.)
void fixupIssueIdsInSourceFiles(
    const vector<CommentBlock *> &commentBlocks,
    map<string, vector<string> > &linesByFile) {

    for(unsigned int i = 0; i < commentBlocks.size(); i++) {

        // Skip over normal boring comments.
        if(commentBlocks[i]->type == BLOCKTYPE_NONE)
            continue;

        // Skip over anything marked as "done".
        if(commentBlocks[i]->type == BLOCKTYPE_DONE)
            continue;

        if(commentBlocks[i]->needsNewId) {

            // TODO: Generate issue id.

            // If we didn't have an issue already, we need to
            // output a modified line for this.

            unsigned int startLength = getStartLengthForBlockType(commentBlocks[i]->type);
            ostringstream fixedLine;
            string filename = commentBlocks[i]->filename;
            string origLine = linesByFile[filename][commentBlocks[i]->startLineNumber];
            fixedLine <<
                origLine.substr(
                    0, commentBlocks[i]->startPositionInLine + startLength) <<
                " [" << commentBlocks[i]->issueId << "]" <<
                linesByFile[filename][commentBlocks[i]->startLineNumber].substr(
                    commentBlocks[i]->startPositionInLine + startLength);

            linesByFile[filename][commentBlocks[i]->startLineNumber] = fixedLine.str();

            commentBlocks[i]->needsNewId = false;
        }
    }
}

void outputOrgFile(
    vector<CommentBlock *> &comments) {

    map<string, vector<CommentBlock *> > commentBlocksByFile;
    for(unsigned int i = 0; i < comments.size(); i++) {
        commentBlocksByFile[comments[i]->filename].push_back(comments[i]);
    }

    for(map<string, vector<CommentBlock *> >::iterator i = commentBlocksByFile.begin();
        i != commentBlocksByFile.end(); i++) {

        vector<CommentBlock *> &commentBlocks = (*i).second;
        string filename = (*i).first;

        // Count completed and incomplete issues.
        unsigned int numIncomplete = 0;
        unsigned int numComplete = 0;
        for(unsigned int i = 0; i < commentBlocks.size(); i++) {
            switch(commentBlocks[i]->type) {
                case BLOCKTYPE_FIXME:
                case BLOCKTYPE_TODO:
                    numIncomplete++;
                    break;
                case BLOCKTYPE_DONE:
                    numComplete++;
                    break;
                default:
                    break;
            }
        }

        // Begin file section.
        cout << endl;
        cout << "* [" << numComplete << "/" << (numIncomplete + numComplete) << "] " << filename << endl;
        // cout << "* " << filename << endl;

        for(unsigned int i = 0; i < commentBlocks.size(); i++) {

            if(commentBlocks[i]->type == BLOCKTYPE_NONE)
                continue;

            // Get JUST the comment.
            string commentWithoutJunk;
            unsigned int startOfActualComment = 0;
            getIssueIdNumber(commentBlocks[i]->comment, &startOfActualComment);
            commentWithoutJunk = commentBlocks[i]->comment.substr(startOfActualComment);

            // Squish newlines together.
            ostringstream commentWithoutNewlines;
            vector<string> commentLines;
            stringTokenize(commentWithoutJunk, "\n", commentLines);
            // for(unsigned int i = 0; i < commentLines.size(); i++) {
            //     if(commentLines[i].size()) {
            //         commentWithoutNewlines << commentLines[i];
            //         if(i != commentLines.size() - 1)
            //             commentWithoutNewlines << " ";
            //     }
            // }

            // cout << "----------------------------------------------------------------------" << endl;
            // cout << "Comment id: " << i << endl;
            // cout << "Issue id:   " << commentBlocks[i]->issueId << endl;
            // cout << "Type:       " << commentBlocks[i]->type << endl;
            // cout << "Line:       " << commentBlocks[i]->startLineNumber << endl;
            // // cout << commentBlocks[i]->comment << endl;
            // cout << commentWithoutJunk << endl;

            // Determine appropriate output block type.
            string typeStr = "TODO";
            if(commentBlocks[i]->type == BLOCKTYPE_DONE)
                typeStr = "DONE";

            // Output just the first line (whole paragraph block in
            // source) as the TODO issue title.
            cout << "** " << typeStr << " [" << commentBlocks[i]->issueId << "] " << (commentLines.size() ? commentLines[0] : "")  << endl;

            // Output all additional lines with some silly tag.
            for(unsigned int j = 1; j < commentLines.size(); j++) {
                cout << extraDataReadOnlyTag << commentLines[j] << endl;
            }

            // Create an org-mode link straight to this comment.
            cout << extraDataReadOnlyTag <<
                "[[file:" <<
                commentBlocks[i]->filename <<
                "::" <<
                (commentBlocks[i]->startLineNumber + 1) <<
                "][link to comment]]" <<
                endl;

            // Create a header for all the notes associated with this.
            if(commentBlocks[i]->extraData.size()) {
                cout << extraDataNotesHeader << endl;
            }

            // Spit out all the stuff that we previously read from the
            // org-mode file.
            for(unsigned int j = 0; j < commentBlocks[i]->extraData.size(); j++) {
                cout << commentBlocks[i]->extraData[j] << endl;
            }

            // Arbitrary newline for readability.
            cout << endl;
        }
    }
}


static inline bool lineEndsLastBlock(const std::string &str) {
    return strStartsWith("** ", str) ||
        strStartsWith("* ", str);
}

void processOrgFile(
    const std::string &filename,
    vector<string> &lines,
    vector<CommentBlock*> &commentBlocks) {

    string currentSourceFile;
    BlockType type = BLOCKTYPE_NONE;
    int issueId = -1;
    vector<string> extraDataLines;
    string commentLine;

    for(unsigned int lineNum = 0; lineNum < lines.size() + 1; lineNum++) {

        // Note: This can be an invalid reference. Don't use it
        // without first testing lineNum < lines.size().
        string &line = lines[lineNum];

        // Finish the current extra data stuff.
        if(lineNum == lines.size() || lineEndsLastBlock(line)) {

            if(issueId != -1) {

                // Find the comment block in the list of comment
                // blocks.
                unsigned int i;
                for(i = 0; i < commentBlocks.size(); i++) {

                    if(commentBlocks[i]->issueId == issueId) {

                        // Found it. Stick the extra data on it.
                        commentBlocks[i]->extraData = extraDataLines;
                        break;
                    }
                }

                if(i == commentBlocks.size()) {

                    // Made it to the end without finding anything.
                    // Must be one of the DONE comments. One way or
                    // another, we need to make a new one.
                    CommentBlock *newBlock = new CommentBlock();
                    newBlock->startLineNumber = 0;
                    newBlock->startPositionInLine = 0;
                    newBlock->issueId = issueId;
                    newBlock->comment = commentLine;
                    newBlock->filename = currentSourceFile;
                    newBlock->type = BLOCKTYPE_DONE;
                    newBlock->extraData = extraDataLines;
                    newBlock->needsNewId = true;
                    commentBlocks.push_back(newBlock);
                }
            }

            // Now cleanup so we can start again.
            issueId = -1;
            extraDataLines.clear();
            type = BLOCKTYPE_NONE;
            commentLine = "";
        }

        if(lineNum < lines.size()) {

            // Skip any line beginning with our "extra" tag. We'll
            // just regenerate that from source anyway.
            if(strStartsWith(extraDataReadOnlyTag, line)) {
                continue;
            }

            // Skip the notes section header we stick in for
            // organization.
            if(strStartsWith(extraDataNotesHeader, line)) {
                continue;
            }

            if(strStartsWith("* ", line)) {

                // Find the first space after the number of
                // open/closed issues.
                unsigned int filenameStart = 2;
                while(filenameStart < line.size() && line[filenameStart] != ' ') {
                    filenameStart++;
                }

                // Skip the space too.
                if(filenameStart < line.size()) {
                    filenameStart++;
                }

                currentSourceFile = line.substr(filenameStart);
                cout << "switching source file: " << currentSourceFile << endl;

            } else if(strStartsWith("** ", line)) {

                string justLine = line.substr(3);
                BlockType type = getBlockType(line);
                unsigned int lineStartPos = 0;
                issueId = getIssueIdNumber(justLine, &lineStartPos);

                justLine = justLine.substr(lineStartPos);
                commentLine = justLine;

            // } else if(!strStartsWith("*", line)) {
            } else if(!lineEndsLastBlock(line)) {

                // Must be some notes we stuck in the .org file.
                extraDataLines.push_back(line);

            }
        }

    }

}

void outputFiles(
    map<string, vector<string> > &linesByFile) {

    // TODO: Make this actually output files.

    for(map<string, vector<string> >::iterator i = linesByFile.begin();
        i != linesByFile.end(); i++) {

        cout << "----------------------------------------------------------------------" << endl;
        cout << (*i).first << endl;
        cout << "----------------------------------------------------------------------" << endl;

        vector<string> &lines = (*i).second;
        for(unsigned int j = 0; j < lines.size(); j++) {
            cout << lines[j] << endl;
        }
        cout << endl << endl << endl;

    }
}

void assignMissingIssueNumbers(
    vector<CommentBlock *> &commentBlocks) {

    int maxIssueNumber = 0;

    // First find the highest number.
    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        if(commentBlocks[i]->issueId > maxIssueNumber)
            maxIssueNumber = commentBlocks[i]->issueId + 1;
    }

    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        if(commentBlocks[i]->issueId == -1) {
            commentBlocks[i]->issueId = maxIssueNumber;
            maxIssueNumber++;
        }
    }
}

int main(int argc, char *argv[]) {

    // TODO: Command line parameter usage junk.
    vector<CommentBlock *> commentBlocks;
    map<string, vector<string> > linesByFile;

    unsigned int numCppFiles = 0;
    unsigned int numOrgFiles = 0;
    string resultOrgFile;

    for(unsigned int i = 1; i < argc; i++) {
        string filename = argv[i];
        if(strEndsWith(".cpp", filename)) {
            loadFileLines(filename, linesByFile[filename]);
            processSourceFile(filename, linesByFile[filename], commentBlocks);
            numCppFiles++;
        }
    }

    setBlockTypesByTag(commentBlocks);

    for(unsigned int i = 1; i < argc; i++) {
        string filename = argv[i];
        if(strEndsWith(".org", filename)) {
            loadFileLines(filename, linesByFile[filename]);
            processOrgFile(filename, linesByFile[filename], commentBlocks);
            resultOrgFile = filename;
            numOrgFiles++;
        }
    }

    if(!numCppFiles || numOrgFiles != 1) {
        cerr << "Usage: " << argv[0] << " <source files> <org mode file>" << endl;
    }


    assignMissingIssueNumbers(commentBlocks);

    fixupIssueIdsInSourceFiles(commentBlocks, linesByFile);

    // outputFiles(linesByFile);

    outputOrgFile(commentBlocks);


    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        delete commentBlocks[i];
    }

    return 0;
}

// Dooooooooom!
