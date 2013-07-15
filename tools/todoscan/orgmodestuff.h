#pragma once

#include <iostream>
#include <vector>
#include <string>

struct CommentBlock;

/// Write out an org file.
void outputOrgFile(
    std::ostream &out,
    std::vector<CommentBlock *> &comments);

/// Parse an org file and update the CommentBlocks. Also adds
/// CommentBlocks for issues that weren't found in the source file.
/// This must be run AFTER processing source files because it expects
/// to match issues in the org file to issues found in the source
/// code.
void processOrgFile(
    const std::string &filename,
    std::vector<string> &lines,
    std::vector<CommentBlock*> &commentBlocks);

