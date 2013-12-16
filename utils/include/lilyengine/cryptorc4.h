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

// For size_t
#include <cstring>

namespace ExPop {

    /// Simple RC4 implementation.
    class CryptoRC4 {
    public:

        CryptoRC4(void);

        /// Set the key and initialize the state array. This can be
        /// called multiple times to slightly mitigate some weaknesses
        /// in RC4. (Ciphersaber 2 runs this 20 times.) The key is not
        /// stored in memory.
        void setKey(const void *keyBytes, size_t keyLen);

        /// Encrypt data in-place. Modifies input!
        void encrypt(void *data, size_t length);

    private:

        unsigned char state[256];

    };
}
