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

// std::streambuf based type for reading DEFLATEd data from another
// stream and decompressing on-the-fly.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "deflate.h"

#include <memory>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    namespace Deflate
    {
        // Internal state for DecompressorStreamBuf.
        struct DecompressState;

        /// std::streambuf type. Give it another stream to read and
        /// decompress.
        ///
        /// Supports seeking in the stream, but seeking backwards will
        /// cause the system to decompress all the data up to the new
        /// point. It also requires that the source stream supports
        /// seeking.
        ///
        /// If seeking is used, then the compressed data should start
        /// at the start of the source stream. If you don't intend use
        /// seeking, this is not a requirement and it can start in the
        /// middle of a stream. ExPop::StreamSection can help deal
        /// with stream offset issues.
        ///
        /// Recommended that you use this with ExPop::StreamSection to
        /// wrap the source stream.
        class DecompressorStreamBuf : public std::streambuf
        {
        public:

            /// Constructor. Takes a pointer to another stream as a
            /// parameter. This should be the only thing reading from
            /// the stream until the object is no longer needed.
            DecompressorStreamBuf(std::istream &inSource);

            /// Shared-ownership constructor.
            DecompressorStreamBuf(std::shared_ptr<std::istream> inSource);

            virtual ~DecompressorStreamBuf();

        protected:

            void initialize();

            /// Reset to the start of the stream.
            void reset();

            // ----------------------------------------------------------------------
            // std::streambuf interface.

            int underflow() override;
            int uflow() override;
            std::streamsize showmanyc() override;
            int sync() override;
            int pbackfail(int c = EOF) override;

            std::streampos seekoff(
                std::streamoff off,
                std::ios_base::seekdir way,
                std::ios_base::openmode which) override;

            std::streampos seekpos(
                std::streampos sp,
                std::ios_base::openmode which) override;

        private:

            // Current offset in the decompressed data. NOT a position
            // in the source stream.
            size_t readPosition;

            // Source stream.
            std::shared_ptr<std::istream> source;

            // Internal decompression state.
            DecompressState *state;

            // This will be the next value that the stream returns.
            // It's always one byte ahead of the actual returned data.
            // This is to handle how underflow() can read data without
            // advancing the read pointer, and so that we can apply
            // finishing touches to the stream when we read the last
            // byte without having to see an EOF first.
            int32_t lastValueRead;

            // End-of-stream indicator.
            bool reachedEof;
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
        inline void DecompressorStreamBuf_fakeDeleter(void *junk)
        {
        }

        inline DecompressorStreamBuf::DecompressorStreamBuf(
            std::istream &inSource)
        {
            state = nullptr;
            source = std::shared_ptr<std::istream>(
                &inSource, DecompressorStreamBuf_fakeDeleter);
            initialize();
        }

        inline DecompressorStreamBuf::DecompressorStreamBuf(
            std::shared_ptr<std::istream> inSource)
        {
            state = nullptr;
            source = inSource;
            initialize();
        }

        inline DecompressorStreamBuf::~DecompressorStreamBuf()
        {
            delete state;
        }

        inline void DecompressorStreamBuf::initialize()
        {
            readPosition = 0;
            reachedEof = false;
            lastValueRead = EOF;

            // Clean up old state.
            if(state) {
                delete state;
            }

            // Make new state.
            state = new DecompressState(*source);

            // Pump the first byte into our buffer.
            lastValueRead = readNextValue(*state);
        }

        inline void DecompressorStreamBuf::reset()
        {
            source->seekg(0);
            initialize();
        }

        inline int DecompressorStreamBuf::underflow()
        {
            if(reachedEof) {
                return EOF;
            }

            // FIXME: Don't think we need this anymore.
            if(lastValueRead == EOF) {
                return uflow();
            }

            return lastValueRead;
        }

        inline int DecompressorStreamBuf::uflow()
        {
            if(reachedEof) {
                return EOF;
            }

            int32_t ret = lastValueRead;

            int32_t nextValue = readNextValue(*state);

            if(nextValue == EOF) {
                reachedEof = true;
                state->bitStream.dropBitsToByteBoundary();
            } else {
                lastValueRead = nextValue;
            }
            readPosition++;

            return ret;
        }

        inline std::streamsize DecompressorStreamBuf::showmanyc()
        {
            if(reachedEof) {
                return 0;
            }

            return 1;
        }

        inline int DecompressorStreamBuf::sync()
        {
            // I wanted to restore the read pointer here, but I
            // confused the decompressed data read pointer with the
            // source stream's read pointer. We would need to preserve
            // the actual read pointer from the BitVector's dataStream
            // member in order to restore the read pointer, anyway.

            // But it's okay, we can rely on the StreamSection's
            // synchronization system to deal with this, just as long
            // as StreamSections only have one reader (which is this).

            // We'll let the multi-reader aspect be the
            // StreamSection's job.

            return 0;
        }

        inline int DecompressorStreamBuf::pbackfail(int c)
        {
            return EOF;
        }

        inline std::streampos DecompressorStreamBuf::seekoff(
            std::streamoff off,
            std::ios_base::seekdir way,
            std::ios_base::openmode which)
        {
            std::streamoff newPos = readPosition;

            switch(way) {

                case std::ios_base::beg:
                    newPos = off;
                    break;

                case std::ios_base::cur:
                    newPos += off;
                    break;

                case std::ios_base::end:
                default:
                    // Error: Seeking to some offset from the end is
                    // unsupported because we don't actually know
                    // where the end is until we're done
                    // decompressing.
                    return -1;
            }

            if(newPos < 0) {
                // Error: Seeking to something before the start of the
                // stream.
                return -1;
            }

            return seekpos(newPos, std::ios_base::in);
        }

        inline std::streampos DecompressorStreamBuf::seekpos(
            std::streampos sp,
            std::ios_base::openmode which)
        {
            // Can't seek to anywhere before the start of the stream.
            if(sp < 0) {
                return -1;
            }

            // We have to completely restart decompression if the byte
            // is from before the current read pointer, because we
            // don't cache enough data to go back.
            if((size_t)sp < readPosition) {
                reset();
            }

            // Skip bytes up until we hit the desired read pointer or
            // an EOF.
            while(readPosition < (size_t)sp) {

                int32_t nextValue = readNextValue(*state);

                if(nextValue != EOF) {
                    lastValueRead = nextValue;
                }

                // FIXME: Should this be after the EOF break?
                readPosition++;

                if(nextValue == EOF) {
                    break;
                }
            }

            return readPosition;
        }
    }
}

