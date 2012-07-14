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

namespace ExPop {

    // All of these compression functions kind of suck. You should
    // probably just use zlib like anyone who's sane. (They work okay
    // for Lily game tilemaps, though.)

    /// Compress a chunk of data. Compressed data will be returned and
    /// is owned by the caller (must eventually delete[] it). Length
    /// of the returned chunk is stored in outLength. groupSize is the
    /// number of bytes to be considered a single unit for the
    /// purposes of run-length encoding.
    char *compressRLE(
        const char *inData,
        unsigned int inLength,
        unsigned int *outLength,
        int groupSize = 1);

    /// Same as above, but takes a previously compressed chunk and
    /// returns a decompressed chunk. Make sure to use the same
    /// groupSize for decompression. If decompression fails due to
    /// corrupt data or incorrect groupSize, will return NULL. (Or
    /// possibly just crash. :D)
    char *decompressRLE(
        const char *inData,
        unsigned int inLength,
        unsigned int *outLength,
        int groupSize = 1);

}

