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

#include "cryptorc4.h"

namespace ExPop {

    CryptoRC4::CryptoRC4(void) {
        for(unsigned int i = 0; i < 256; i++) {
            state[i] = i;
        }
    }

    static inline void swapChar(unsigned char &a, unsigned char &b) {

        // For some reason, the XOR swap trick isn't working. What the
        // hell?

        // a = a ^ b;
        // b = a ^ b;
        // a = a ^ b;

        // Fall back on old derpy temp variable.

        unsigned char tmp = a;
        a = b;
        b = tmp;
    }

    void CryptoRC4::setKey(const void *keyBytes, size_t keyLen) {

        unsigned int j = 0;

        for(unsigned int i = 0; i < 256; i++) {
            // unsigned int keyIndex = i % keyLen;
            unsigned char keyVal =
                ((const unsigned char*)keyBytes)[i % keyLen];

            j = (int(j) + int(state[i]) + int(keyVal)) % 256;
            swapChar(state[j], state[i]);
        }
    }

    void CryptoRC4::encrypt(void *data, size_t length) {

        unsigned int i = 0;
        unsigned int j = 0;
        unsigned int msgPt = 0;

        do {
            i = (i + 1) % 256;
            j = (j + state[i]) % 256;

            swapChar(state[i], state[j]);

            unsigned int n = state[i] + state[j];

            unsigned char *dataPtr = ((unsigned char*)data) + msgPt;
            *dataPtr = *dataPtr ^ state[n % 256];

            msgPt++;

        } while (msgPt < length);
    }

}

