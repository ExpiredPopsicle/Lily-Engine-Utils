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

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cstring>
using namespace std;

#include <lilyengine/malstring.h>
using namespace ExPop;

#include "common.h"
#include "orgmodestuff.h"

void outputOrgFile(
    std::ostream &out,
    std::vector<CommentBlock *> &comments) {

    map<string, vector<CommentBlock *> > commentBlocksByFile;
    for(unsigned int i = 0; i < comments.size(); i++) {
        cout << "Pushing comment: " << comments[i]->type << " " << comments[i]->issueId << endl;
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
        out << "* [" << numComplete << "/" << (numIncomplete + numComplete) << "] " << filename << endl;

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

            // Determine appropriate output block type.
            string typeStr = "TODO";
            if(commentBlocks[i]->type == BLOCKTYPE_DONE)
                typeStr = "DONE";

            // Org-modes doesn't recognize "FIXME"s, so just jam
            // it in after all the important junk, if necessary.
            string fixmeAddin = "";
            if(commentBlocks[i]->type == BLOCKTYPE_FIXME) {
                fixmeAddin = "FIXME: ";
            }

            // Output just the first line (whole paragraph block in
            // source) as the TODO issue title.
            out << "** " << typeStr << " [" << commentBlocks[i]->issueId << "] " <<
                fixmeAddin <<
                (commentLines.size() ? strTrim(commentLines[0]) : "")  << endl;

            // Output all additional lines with some silly tag.
            for(unsigned int j = 1; j < commentLines.size(); j++) {

                // Output a prefixed, indented, wordwrapped thingy.
                out <<
                    strPrefixLines(
                        strIndent(
                            strWordWrap(
                                fixmeAddin + commentLines[j],
                                60),
                            0,
                            2),
                        extraDataReadOnlyTag) << endl;
            }

            // Create an org-mode link straight to this comment.
            out << extraDataReadOnlyTag <<
                "[[file:" <<
                commentBlocks[i]->filename <<
                "::" <<
                (commentBlocks[i]->startLineNumber + 1) <<
                "][link to comment]]" <<
                endl;

            // Create a header for all the notes associated with this.
            if(commentBlocks[i]->extraData.size()) {
                out << extraDataNotesHeader << endl;
            }

            // Spit out all the stuff that we previously read from the
            // org-mode file.
            for(unsigned int j = 0; j < commentBlocks[i]->extraData.size(); j++) {

                // Skip whitespace near the end, because it gets added
                // by the org output thingy.
                if(j == commentBlocks[i]->extraData.size() - 1 && commentBlocks[i]->extraData[j] == "")
                    continue;

                out << commentBlocks[i]->extraData[j] << endl;
            }

            // Arbitrary newline for readability.
            out << endl;
        }
    }
}


static inline bool lineEndsLastBlock(const std::string &str) {
    return strStartsWith("** ", str) ||
        strStartsWith("* ", str);
}

void processOrgFile(
    const std::string &filename,
    std::vector<std::string> &lines,
    vector<CommentBlock*> &commentBlocks) {

    string currentSourceFile;
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

            } else if(strStartsWith("** ", line)) {

                string justLine = line.substr(3);
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

