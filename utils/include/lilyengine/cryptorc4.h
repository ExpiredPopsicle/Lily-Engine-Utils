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

// RC4 crypto implementation. You should not use this for anything
// needing any kind of actual security. Really. RC4 is badly broken.
// Makes a nice random number generator, though.

// FIXME: This implementation has not been tested against any examples
// since it was changed to have the randomness generation split off
// from the actual encryption.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <cstring> // For size_t and memset

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Simple RC4 implementation.
    class CryptoRC4
    {
    public:

        CryptoRC4();
        ~CryptoRC4();

        /// Set the key and initialize the state array.
        void setKey(const void *keyBytes, size_t keyLen);

        /// Simpler std::string version for extra terrible security.
        void setKey(const std::string &key);

        /// Encrypt data in-place. Modifies input! Note: RC4 is a
        /// symmetric cipher, so this function also decrypts.
        void encrypt(void *data, size_t length);

        /// Encrypt a string and return the encrypted string. Note:
        /// RC4 is a / symmetric cipher, so this function also
        /// decrypts.
        std::string encrypt(const std::string &str);

        /// Just get the random values that would normally be XORed to
        /// encrypt or decrypt. Gets one byte of randomness.
        unsigned char getRandomness();

        /// This gets a random integer of some integer type. Doesn't
        /// care about sign bits so you probably want to use with
        /// unsigned types only.
        template<typename T>
        T getRandomIntegral();

    private:

        uint32_t state_i;
        uint32_t state_j;
        unsigned char state[256];

    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline CryptoRC4::CryptoRC4()
    {
        for(unsigned int i = 0; i < 256; i++) {
            state[i] = i;
        }
        state_i = 0;
        state_j = 0;
    }

    inline CryptoRC4::~CryptoRC4()
    {
        state_i = 0;
        state_j = 0;
        std::memset(state, 0, sizeof(state));
    }

    static inline void swapChar(unsigned char &a, unsigned char &b)
    {
        // The XOR swap trick isn't working, because sometimes we're
        // passed the same reference for both parameters. It's here in
        // case we ever want to fix that.

        // a = a ^ b;
        // b = a ^ b;
        // a = a ^ b;

        // Fall back on old derpy temp variable.

        unsigned char tmp = a;
        a = b;
        b = tmp;
    }

    // FIXME: Calling this multiple times will not correctly add to
    // the key, because we don't keep track of the j variable between
    // runs.
    inline void CryptoRC4::setKey(const void *keyBytes, size_t keyLen)
    {
        unsigned int j = 0;

        for(unsigned int i = 0; i < 256; i++) {

            unsigned char keyVal =
                ((const unsigned char*)keyBytes)[i % keyLen];

            j = (int(j) + int(state[i]) + int(keyVal)) % 256;
            swapChar(state[j], state[i]);
        }
    }

    inline void CryptoRC4::setKey(const std::string &key)
    {
        setKey(&key[0], key.size());
    }

    inline unsigned char CryptoRC4::getRandomness()
    {
        state_i = (state_i + 1) % 256;
        state_j = (state_j + state[state_i]) % 256;

        swapChar(state[state_i], state[state_j]);

        unsigned int n = state[state_i] + state[state_j];

        return state[n % 256];
    }

    inline void CryptoRC4::encrypt(void *data, size_t length)
    {
        for(size_t i = 0; i < length; i++) {
            unsigned char *dataPtr = ((unsigned char*)data) + i;
            *dataPtr = *dataPtr ^ getRandomness();
        }
    }

    inline std::string CryptoRC4::encrypt(const std::string &str)
    {
        std::string ret;
        ret.resize(str.size());
        for(size_t i = 0; i < str.size(); i++) {
            ret[i] = str[i] ^ getRandomness();
        }
        return ret;
    }

    template<typename T>
    inline T CryptoRC4::getRandomIntegral()
    {
        static_assert(
            std::is_integral<T>::value,
            "Some kind of integer-like thing required.");
        T val;
        for(size_t i = 0; i < sizeof(T); i++) {
            *(((unsigned char*)&val) + i) = getRandomness();
        }
        return val;
    }
}



