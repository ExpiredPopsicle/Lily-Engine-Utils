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

// TODO: Herp some derps.

// TODO: More complicated blocks, sometimes spanning multiple lines!
//   (This is not actually a TODO. I'm actually testing that feature
//   with this very comment block. Derp.)

string testThinger  = "// TODO: Don't read this string";
string testThinger2 = "\" // TODO: Don't read this string either";

struct CommentBlock {
    int startLineNumber;
    string comment;
};

int main(int argc, char *argv[]) {

    string fileName = argv[1];
    int fileLen = 0;
    char *fileData = FileSystem::loadFile(fileName, &fileLen, true);

    ostringstream currentBlockOut;
    int blockStartLine = -1;
    vector<CommentBlock *> commentBlocks;
    bool lastCommentLineWasNewline = false;

    if(fileData) {

        // Separate file into lines.
        vector<string> lines;
        stringTokenize(fileData, "\n", lines, true);

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

                // // Skip past any whitespace.
                // while(charNum < lines[lineNum].size() && isWhiteSpace(lines[lineNum][charNum])) {
                //     charNum++;
                // }

                string commentStr = strTrim(lines[lineNum].substr(charNum));

                // cout << commentStr << endl;

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

                    // First line that isn't a comment after a
                    // block. Finish up the block and add it to the
                    // list.
                    //
                    // Herp derp

                    CommentBlock *newBlock = new CommentBlock();
                    newBlock->startLineNumber = blockStartLine;
                    newBlock->comment = currentBlockOut.str();
                    commentBlocks.push_back(newBlock);

                    blockStartLine = -1;
                    currentBlockOut.str("");
                }
            }

        }


        if(blockStartLine != -1) {

            // Finish off the last comment block.

            CommentBlock *newBlock = new CommentBlock();
            newBlock->startLineNumber = blockStartLine;
            newBlock->comment = currentBlockOut.str();
            commentBlocks.push_back(newBlock);

            blockStartLine = -1;
            currentBlockOut.str("");
        }


        delete[] fileData;


        // TODO: Do something useful with this.
        for(unsigned int i = 0; i < commentBlocks.size(); i++) {
            if(strStartsWith("TODO", commentBlocks[i]->comment) || strStartsWith("FIXME", commentBlocks[i]->comment)) {
                cout << "----------------------------------------------------------------------" << endl;
                cout << "Comment id: " << i << endl;
                cout << "Line:       " << commentBlocks[i]->startLineNumber << endl;
                cout << commentBlocks[i]->comment << endl;
            }
            delete commentBlocks[i];
        }


    } else {

        cout << "Error reading file: " << fileName << endl;

    }

    return 0;
}


// Dooooooooom!
