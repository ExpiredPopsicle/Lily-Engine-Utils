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

// This is just all the source files together for the library in case
// I want to be lazy and only name one file in a project. Don't build
// it as part of a project with the cpp files themselves unless you
// want lots of redefined symbols all over the place.

// These ones depend on or implement the thread module in some way...

// #include "thread/thread.cpp"
// #include "filesystem/archive.cpp"
// #include "filesystem/filesystem.cpp"
// #include "console/console.cpp"
// #include "parser/parser.cpp"
// #include "parser/parserxml.cpp"
// #include "assetloader/assetloader.cpp"
// #include "preprocess/preprocess.cpp"

// Completely independent systems...

#include "string/base64.cpp"
// FIXME: These two stolen by DerpScript.
#include "string/string.cpp"
// #include "string/pooledstring.cpp"
// #include "assert/malassert.cpp"
#include "compress/compress.cpp"
#include "math/matrix.cpp"
#include "refsystem/refsystem.cpp"
#include "crypto/cryptorc4.cpp"
#include "parser/parser.cpp"
#include "parser/parserxml.cpp"
