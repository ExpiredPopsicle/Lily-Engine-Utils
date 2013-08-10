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

    const unsigned int DERP_MAX_TOKEN_LENGTH = 1024;
    const unsigned int DERP_MAX_STRING_LENGTH = 1024;
    const unsigned int DERP_MAX_TOKENS = 65536;

    // FIXME: We can probably take out the per-function call counter
    // now that we have a global call count thing.
    const unsigned int DERP_MAX_STACK_FRAMES = 16384;

    const unsigned int DERP_MAX_OBJECT_COUNT = 0xefffffff;

}

