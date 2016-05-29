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

#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>


namespace ExPop
{
    /// Simple buffer that we can dynamically expand or iterate through.
    class SimpleBuffer {
	public:

		SimpleBuffer(void);
		SimpleBuffer(const void *data, size_t length, bool myOwnData = true);
		~SimpleBuffer(void);

        /// Add some data onto the end. Reallocates if needed. Note:
        /// Doesn't just double the size of the buffer. It only
        /// allocates as much as it needs so continuously adding to
        /// the buffer in this way can be slow.
		void addData(const void *buffer, size_t length);

        /// Just get a pointer to the start of the data.
		const char *getData(void) const;

        /// Get the current length of the data.
		size_t getLength(void) const;

        /// Reads a number of bytes equal to the bytes parameter into
        /// the memory pointed to by dst and advances the read pointer
        /// by that amount. Returns a pointer to the position the read
        /// pointer ends at.
		const char *getDataAndAdvance(void *dst, size_t bytes);

        /// Moves the read pointer back to the start of the data.
        void seekToStart(void);

        int compare(const void *buffer, size_t length);

        void clear(void);

	private:

        // If this is true, the data will be cleaned up when this goes
        // away.
        bool myOwnData;

		char *data;
		size_t length;
		size_t readPtr;
    };
}
