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

// std::streambuf derived type to represent reading a subsection of
// another stream. If the source stream does not support seeking, then
// this one will not either. It WILL move the read position of the
// source stream as the subsection object is read.

// If seeking is supported in the source stream, then it will force
// the read pointer in the source to be where the StreamSection thinks
// it should be. This allows multiple StreamSection objects to use the
// same std::istream or derived type as a source without interfering
// with each other's reads.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <iostream>
#include <memory>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Wrapper for a section of another stream.
    class StreamSection : public std::streambuf
    {
    public:

        /// Non-ownership version. We'll still use std::shared_ptr
        /// internally for this, but with a fake deletion function
        /// that does nothing. This should be safe for streams
        /// declared on the stack.
        StreamSection(
            std::istream &inSource,
            size_t length);

        /// Shared-ownership version.
        StreamSection(
            std::shared_ptr<std::istream> inSource,
            size_t length);

    private:

        // Offsets inside source stream.
        size_t startOffset;
        size_t endOffset;

        // Offset inside this sub-section.
        size_t currentOffsetInSection;

        // Source istream.
        std::shared_ptr<std::istream> source;

        // Common initialization for all constructors.
        void commonInit(size_t length);

    protected:

        // std::streambuf interface.
        int underflow() override;
        int uflow() override;
        std::streamsize showmanyc() override;
        int sync() override;

        std::streampos seekoff(
            std::streamoff off,
            std::ios_base::seekdir way,
            std::ios_base::openmode which) override;

        std::streampos seekpos(
            std::streampos sp,
            std::ios_base::openmode which) override;

        std::streampos getCurrentSourceOffset();
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{

    inline std::streampos StreamSection::seekoff(
        std::streamoff off,
        std::ios_base::seekdir way,
        std::ios_base::openmode which)
    {
        std::streamoff newPos = currentOffsetInSection;
        switch(way) {
            case std::ios_base::beg:
                newPos = off;
                break;
            case std::ios_base::cur:
                newPos += off;
                break;
            case std::ios_base::end:
                newPos = (endOffset - startOffset) - off;
                break;
            default:
                return -1;
        }

        if(newPos < 0) {
            // Error: Attempt to seek before beginning of stream.
            return -1;
        }

        if((size_t)newPos > (endOffset - startOffset)) {
            // Error: Attempt to seek past end of stream.
            return -1;
        }

        currentOffsetInSection = newPos;

        return currentOffsetInSection;
    }

    inline std::streampos StreamSection::seekpos(
        std::streampos sp,
        std::ios_base::openmode which)
    {
        if(sp < 0) {
            // Error: Attempt to seek before beginning of stream.
            return -1;
        }

        if((size_t)sp > (endOffset - startOffset)) {
            // Error: Attempt to seek past end of stream.
            return -1;
        }

        currentOffsetInSection = sp;

        return currentOffsetInSection;
    }

    inline std::streampos StreamSection::getCurrentSourceOffset()
    {
        return source->tellg();
    }

    inline void StreamSection_fakeDeleter(void *junk)
    {
    }

    inline void StreamSection::commonInit(size_t length)
    {
        endOffset = length;

        // Find the starting offset in the source stream, if that
        // stream supports seeking. Otherwise use zero.
        std::streampos sourceOffset = getCurrentSourceOffset();
        if(sourceOffset != -1) {
            startOffset = sourceOffset;
        } else {
            startOffset = 0;
        }
        endOffset = startOffset + length;

        currentOffsetInSection = 0;
    }

    inline StreamSection::StreamSection(
        std::istream &inSource,
        size_t length)
    {
        source = std::shared_ptr<std::istream>(&inSource, StreamSection_fakeDeleter);
        commonInit(length);
    }

    inline StreamSection::StreamSection(
        std::shared_ptr<std::istream> inSource,
        size_t length)
    {
        source = inSource;
        commonInit(length);
    }

    inline int StreamSection::underflow()
    {
        sync();
        if(startOffset + currentOffsetInSection < endOffset) {
            return source->peek();
        }
        return EOF;
    }

    inline int StreamSection::uflow()
    {
        sync();
        if(startOffset + currentOffsetInSection < endOffset) {
            currentOffsetInSection++;
            return source->get();
        }
        return EOF;
    }

    inline std::streamsize StreamSection::showmanyc()
    {
        sync();
        size_t bytesRemaining = (endOffset - (startOffset + currentOffsetInSection));
        return bytesRemaining;
    }

    inline int StreamSection::sync()
    {
        source->sync();

        // Bail out if we don't support seeking in this stream.
        std::streampos sourceOffset = getCurrentSourceOffset();
        if(sourceOffset == -1) {
            return -1;
        }

        // Move the actual read pointer up to where we think we should
        // be.
        std::streamoff diff = (startOffset + currentOffsetInSection) - sourceOffset;
        if(diff != 0) {
            source->seekg(diff, std::ios_base::cur);
        }

        return getCurrentSourceOffset() - std::streampos(startOffset);
    }

}

