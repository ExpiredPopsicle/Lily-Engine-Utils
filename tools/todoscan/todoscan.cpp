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

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std;

#include <lilyengine/utils.h>
using namespace ExPop;

#include "orgmodestuff.h"
#include "common.h"

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

// TODO [101]: First issue.

// TODO [101]: Second issue.

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

// TODO: Determine what happens when multiple comments have the same
// issue id number.

// TODO: Recognize indented blocks for extra data.

// TODO: Some of the lines in the org file are ending up with extra
// spaces.

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

// Iterate over all the blocks in the list, and set the "type" field
// based on the text in the block.
void setBlockTypesByTag(vector<CommentBlock*> &commentBlocks) {

    for(unsigned int i = 0; i < commentBlocks.size(); i++) {

        BlockType blockType = BLOCKTYPE_NONE;

        blockType = getBlockType(commentBlocks[i]->comment);

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
    int indentAmountForLastComment = -1;

    for(unsigned int lineNum = 0; lineNum < lines.size(); lineNum++) {

        // Find the start of a comment
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

            // TODO: Add different comment start things for different
            // source types.
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

            // TODO: Different comment start lengths for different
            // types.

            // Skip the comment marker.
            charNum += 2;

            if(blockStartPosInLine == -1) {

                // Find the actual start of the line after all the
                // whitespace.
                blockStartPosInLine = charNum;

                while(
                    blockStartPosInLine < int(lines[lineNum].size()) &&
                    isWhiteSpace(lines[lineNum][blockStartPosInLine])) {

                    blockStartPosInLine++;
                }
            }

            // Skip past any whitespace. Keep track of the amount of
            // indentation, so we can determine if we want to split
            // into an ext thing.
            int indentAmountForThisComment = 0;
            while(charNum < lines[lineNum].size() && isWhiteSpace(lines[lineNum][charNum])) {
                charNum++;
                indentAmountForThisComment++;
            }

            string commentStr = stringTrim(lines[lineNum].substr(charNum));

            bool addWhiteSpaceToLineStart = !lastCommentLineWasNewline;

            if(!commentStr.size()) {

                // Blank line comment = newline divider thingy. This
                // will get turned into extra data in the org file.
                commentStr = "\n";
                lastCommentLineWasNewline = true;

            } else {

                lastCommentLineWasNewline = false;
            }

            if(commentStr.size() && (indentAmountForLastComment != -1 && indentAmountForLastComment != indentAmountForThisComment)) {

                // Found a comment on a different indentation level.
                // Begin a new line.
                commentStr = "\n" + commentStr;
            }

            indentAmountForLastComment = indentAmountForThisComment;

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

                indentAmountForLastComment = -1;
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

            // If we didn't have an issue already, we need to output a
            // modified line for this.
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

void outputFiles(
    vector<CommentBlock *> &commentBlocks,
    map<string, vector<string> > &linesByFile,
    GitState &gitState,
    bool forceWrite,
    bool writeToStdout) {

    // TODO: Make this actually output files.

  #ifndef WIN32

    if(!forceWrite) {

        // First, make sure everything we're about to overwrite is
        // actually in version control (or just doesn't yet exist).
        for(map<string, vector<string> >::iterator i = linesByFile.begin();
            i != linesByFile.end(); i++) {

            string filename = (*i).first;

            if(FileSystem::fileExists(filename)) {

                if(gitState.trackedFiles.count(filename) &&
                   !gitState.modifiedFiles.count(filename)) {

                    // We're okay.

                } else {

                    // Uh oh.
                    cerr << "File " << (*i).first << " is either not under revision control, or has local modifications." << endl;
                    cout << "Cowardly refusing to modify this file." << endl;
                    exit(1);
                }
            }
        }
    }

  #endif

    for(map<string, vector<string> >::iterator i = linesByFile.begin();
        i != linesByFile.end(); i++) {

        string filename = (*i).first;
        ostringstream outStr;


        if(writeToStdout) {
            cout << "----------------------------------------------------------------------" << endl;
            cout << (*i).first << endl;
            cout << "----------------------------------------------------------------------" << endl;
        } else {
            cout << "Writing " << filename << endl;
        }

        if(stringEndsWith<char>(".org", filename)) {

            // Org files get completely regenerated.
            outputOrgFile(outStr, commentBlocks);

        } else {

            // All other source files just get the modified versions
            // spit out.
            vector<string> &lines = (*i).second;
            for(unsigned int j = 0; j < lines.size(); j++) {
                outStr << lines[j] << endl;
            }
        }

        string outData = outStr.str();

        if(writeToStdout) {
            cout << outData << endl;
        } else {
            FileSystem::saveFile(
                filename,
                outData.c_str(),
                outData.size());
        }
    }
}

void assignMissingIssueNumbers(
    vector<CommentBlock *> &commentBlocks) {

    int maxIssueNumber = 0;

    // First find the highest number.
    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        if(commentBlocks[i]->issueId > maxIssueNumber)
            maxIssueNumber = commentBlocks[i]->issueId;
    }
    maxIssueNumber++;

    // Now go through and fix up issue IDs.
    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        if(commentBlocks[i]->issueId == -1) {
            commentBlocks[i]->issueId = maxIssueNumber;
            maxIssueNumber++;
        }
    }
}

void getGitState(GitState &state) {

  #ifndef WIN32

    string curLine;
    int c;
    FILE *gitRunFd = NULL;

    for(int i = 0; i < 2; i++) {

        gitRunFd = popen(i == 0 ? "git ls-files -m" : "git ls-files", "r");

        while((c = fgetc(gitRunFd)) != EOF) {
            if(c == '\n') {
                if(i == 0) {
                    state.modifiedFiles[curLine] = true;
                } else {
                    state.trackedFiles[curLine] = true;
                }
                curLine = "";
            } else {
                char str[2] = { char(c), 0 };
                curLine = curLine + str;
            }
        }

        pclose(gitRunFd);
    }

  #endif

}

bool cmpCommentBlocks(
    const CommentBlock *cb0,
    const CommentBlock *cb1)
{
    if(cb0->type != cb1->type) {
        return cb0->type < cb1->type;
    }

    return cb0->issueId < cb1->issueId;
}

int main(int argc, char *argv[])
{
    // TODO: Command line parameter usage junk.
    vector<CommentBlock *> commentBlocks;
    map<string, vector<string> > linesByFile;

    unsigned int numCppFiles = 0;
    unsigned int numOrgFiles = 0;
    string resultOrgFile;

    bool forceWrite = false;
    bool writeToStdout = false;

    GitState gitState;
    getGitState(gitState);

    // Split command line parameters into arguments and files.
    vector<string> justFileList;
    vector<string> argsList;
    for(unsigned int i = 1; i < (unsigned int)argc; i++) {
        if(strlen(argv[i]) && argv[i][0] == '-') {
            argsList.push_back(argv[i]);
        } else {
            justFileList.push_back(argv[i]);
        }
    }

    // Handle all the arguments.
    for(unsigned int i = 0; i < argsList.size(); i++) {
        if(argsList[i] == "-f") {
            forceWrite = true;
        }
        if(argsList[i] == "-o") {
            writeToStdout = true;
        }
    }

    // Find all the source files.
    for(unsigned int i = 0; i < justFileList.size(); i++) {
        string filename = justFileList[i];
        if(stringEndsWith<char>(".cpp", filename) || stringEndsWith<char>(".h", filename) || stringEndsWith<char>(".c", filename)) {
            loadFileLines(filename, linesByFile[filename]);
            processSourceFile(filename, linesByFile[filename], commentBlocks);
            numCppFiles++;
        }
    }

    setBlockTypesByTag(commentBlocks);

    // Find the org file.
    for(unsigned int i = 0; i < justFileList.size(); i++) {
        string filename = justFileList[i];
        if(stringEndsWith<char>(".org", filename)) {

            // It's okay if an org file doesn't exist. It just means
            // we'll use it as an output.
            if(FileSystem::fileExists(filename)) {

                loadFileLines(filename, linesByFile[filename]);
                processOrgFile(filename, linesByFile[filename], commentBlocks);

            } else {

                // Still need an entry for it, even if it doesn't
                // exist.
                linesByFile[filename].push_back("");
            }

            resultOrgFile = filename;
            numOrgFiles++;
        }
    }

    // Quick sanity check.
    if(!numCppFiles || numOrgFiles != 1) {
        cerr << "Usage: " << argv[0] << " [-f] [-o] <source files> <org mode file>" << endl;
        cerr << "    -f: Force writing files, even if they are not in any version control." << endl;
        cerr << "    -o: Just write everything to stdout instead of any files." << endl;
        exit(1);
    }

    assignMissingIssueNumbers(commentBlocks);

    sort(commentBlocks.begin(), commentBlocks.end(), cmpCommentBlocks);

    fixupIssueIdsInSourceFiles(commentBlocks, linesByFile);

    outputFiles(commentBlocks, linesByFile, gitState, forceWrite, writeToStdout);

    for(unsigned int i = 0; i < commentBlocks.size(); i++) {
        delete commentBlocks[i];
    }

    return 0;
}

// Dooooooooom!
