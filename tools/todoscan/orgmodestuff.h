// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2018 Kiri Jolly
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

