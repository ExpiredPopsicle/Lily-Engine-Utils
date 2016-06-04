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

// Default configuration. Override these values on the command line,
// or before #including the headers.

#pragma once

// Enable threading library. This should be replaced with standard C++
// threads. This system was just a wrapper around pthreads and windows
// threads and is no longer needed now that threads are part of
// standard C++. However, keeping threading optional will allow us to
// continue to target Emscripten.
#ifndef EXPOP_ENABLE_THREADS
#define EXPOP_ENABLE_THREADS 0
#endif

// Sockets have linking weirdness on Windows. Probably a huge mess on
// Emscripten too.
#ifndef EXPOP_ENABLE_SOCKETS
#define EXPOP_ENABLE_SOCKETS 0
#endif

