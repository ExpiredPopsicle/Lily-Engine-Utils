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

#include <malloc.h>
#include <string>

#include <lilyengine/malstring.h>
#include <lilyengine/simplebuffer.h>

namespace ExPop
{
    SimpleBuffer::SimpleBuffer(const void *inData, size_t inLength, bool inMyOwnData)
    {
        this->myOwnData = inMyOwnData;

        this->length = 0;

        if(inMyOwnData) {

            // This object keeps its own copy of the data.
            this->data = (char*)malloc(inLength);
            this->length = inLength;
            memcpy(this->data, inData, inLength);

        } else {

            // Not my own data.

            // FIXME: This just gets rid of the const. Probably a really
            // horrible way to handle this, but whatever.
            memcpy(&this->data, &inData, sizeof(void*));
            this->length = inLength;
        }

        readPtr = 0;
    }

    SimpleBuffer::SimpleBuffer(void)
    {
        myOwnData = true;

        data = NULL;
        length = 0;

        readPtr = 0;
    }

    SimpleBuffer::~SimpleBuffer(void)
    {
        if(myOwnData && data) free(data);
    }

    void SimpleBuffer::clear(void)
    {
        if(myOwnData && data) free(data);
        data = NULL;
    }

    void SimpleBuffer::addData(const void *buffer, size_t inLength)
    {
        assert(myOwnData);

        size_t newLength = inLength + this->length;
        size_t oldLength = this->length;

        if(data) {
            data = (char*)realloc(data, newLength);
        } else {
            data = (char*)malloc(newLength);
        }

        char *dataWritePtr = data + oldLength;

        memcpy(dataWritePtr, buffer, inLength);

        this->length = newLength;
    }

    const char *SimpleBuffer::getData(void) const
    {
        return data;
    }

    const char *SimpleBuffer::getDataAndAdvance(void *dst, size_t bytes)
    {
        if(readPtr >= length) return NULL;

        const char *ret = data + readPtr;

        if(dst) {

            size_t realBytes = bytes;

            if(readPtr + bytes > length) {
                // Trying to run off the end of the buffer.
                realBytes = length - readPtr;
            }

            if(realBytes != bytes) {
                // If we would go off the end of the buffer, pad the ouput
                // with stuff we can't do.
                memset(dst, 0, bytes);
            }

            memcpy(dst, ret, realBytes);
        }

        readPtr += bytes;

        return ret;
    }

    void SimpleBuffer::seekToStart(void)
    {
        readPtr = 0;
    }

    size_t SimpleBuffer::getLength(void) const
    {
        return length;
    }

    int SimpleBuffer::compare(const void *inBuffer, size_t inLength)
    {
        if(inLength < this->length) return 1;
        if(inLength > this->length) return -1;

        const char *cbuf = (const char *)inBuffer;

        for(size_t i = 0; i < inLength; i++) {
            if(data[i] > cbuf[i]) {
                return 1;
            }
            if(data[i] < cbuf[i]) {
                return -1;
            }
        }

        return 0;
    }
}
