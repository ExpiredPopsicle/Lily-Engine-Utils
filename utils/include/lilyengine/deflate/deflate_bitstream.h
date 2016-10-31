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

// This is a type internal to the DEFLATE implementation and not meant
// to be used externally.

// A system to read bits from a stream. DEFLATE doesn't tend to do
// stuff at convenient byte boundaries, so we have to be able to read
// arbitrary bit counts from streams and interpret them as big or
// little endian numbers.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "deflate_common.h"

#include <iostream>

// ----------------------------------------------------------------------
// Internal declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        // BitStream is essentially a wrapper around another stream
        // that makes addressing individual bits easy.
        class BitStream
        {
        public:

            // Constructor. Pass it a source std::istream to read
            // from.
            BitStream(std::istream &in);

            // Make a string based on the actual data that the bits
            // represent. Must be on a byte boundary to call this.
            std::string eatRawData(size_t lengthInBytes);

            // Read the bits as a big-endian number, converted to this
            // system's native byte order.
            template<typename T>
            T eatIntegral_bigEndian(size_t bitCount = sizeof(T) * 8);

            // Read the bits as a little-endian number, converted to
            // this system's native byte order.
            template<typename T>
            T eatIntegral_littleEndian(size_t bitCount = sizeof(T) * 8);

            // Check to see if this is empty (no more remaining bits).
            bool empty() const;

            // Advance the read pointer by some number of bits.
            void dropBits(size_t length);

            // Advance the read pointer until the number of remaining
            // bits is a multiple of 8.
            void dropBitsToByteBoundary();

        private:

            void initDataBuffer(std::istream &in);

            bool getNextBit();

            BitStream(const BitStream &other);
            BitStream &operator=(const BitStream &other);

            void setFromOther(const BitStream &other);

            // Get a subsection of the vector. length is in bits.
            BitStream slice(size_t start, size_t length) const;

            std::istream *dataStream;
            uint8_t byteBuffer;
            size_t byteBufferBitIndex;
        };
    }
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        inline bool systemIsLittleEndian()
        {
            uint32_t a = 0x00000001;
            return ((char*)&a)[0] == 1;
        }

        template<typename T>
        inline T reverseByteOrder(const T &val)
        {
            T val0 = val;
            for(size_t i = 0; i < sizeof(T) / 2; i++) {
                uint8_t tmp = ((uint8_t*)&val0)[i];
                ((uint8_t*)&val0)[i] = ((uint8_t*)&val0)[sizeof(T) - i - 1];
                ((uint8_t*)&val0)[sizeof(T) - i - 1] = tmp;;
            }
            return val0;
        }

        template<typename T>
        inline T littleEndianToNative(const T &val)
        {
            if(systemIsLittleEndian()) {
                return val;
            }
            return reverseByteOrder(val);
        }

        inline BitStream::BitStream(std::istream &in)
        {
            byteBufferBitIndex = 0;
            byteBuffer = 0;
            dataStream = nullptr;
            initDataBuffer(in);
        }

        inline bool BitStream::getNextBit()
        {
            if(byteBufferBitIndex == 0) {
                dataStream->read((char*)&byteBuffer, 1);
            }
            bool bit = !!(byteBuffer & (0x1 << byteBufferBitIndex));
            byteBufferBitIndex++;
            byteBufferBitIndex %= 8;

            return bit;
        }

        inline std::string BitStream::eatRawData(size_t lengthInBytes)
        {
            // We must be byte-aligned before we can call this.
            EXPOP_DEFLATE_ASSERT(byteBufferBitIndex == 0);

            std::string ret;
            ret.resize(lengthInBytes);
            for(size_t i = 0; i < lengthInBytes; i++) {
                ret[i] = eatIntegral_bigEndian<uint8_t>();
            }

            return ret;
        }

        template<typename T>
        inline T BitStream::eatIntegral_littleEndian(size_t bitCount)
        {
            return reverseByteOrder(eatIntegral_bigEndian<T>(bitCount));
        }

        template<typename T>
        inline T BitStream::eatIntegral_bigEndian(size_t bitCount)
        {
            T ret = 0;

            for(size_t i = 0; !empty() && i < sizeof(T) * 8 && i < bitCount; i++) {
                if(getNextBit()) {
                    ret |= (1 << i);
                }
            }

            return ret;
        }

        inline void BitStream::initDataBuffer(std::istream &in)
        {
            dataStream = &in;
            byteBufferBitIndex = 0;
            byteBuffer = 0;
        }

        inline bool BitStream::empty() const
        {
            if(byteBufferBitIndex == 0) {
                return !dataStream->good();
            }
            return false;
        }

        inline void BitStream::dropBits(size_t length)
        {
            while(length) {

                if(byteBufferBitIndex == 0) {
                    dataStream->read((char*)&byteBuffer, 1);
                }

                byteBufferBitIndex++;
                byteBufferBitIndex %= 8;
                length--;

            }
        }

        inline void BitStream::dropBitsToByteBoundary()
        {
            size_t bitsToDrop = (8 - byteBufferBitIndex) & 0x7;
            dropBits(bitsToDrop);
        }

    }
}
