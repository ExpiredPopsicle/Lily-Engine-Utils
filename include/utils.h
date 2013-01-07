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

#include "winhacks.h"
#include "console.h"
#include "malstring.h"
#include "filesystem.h"
#include "thread.h"
#include "refsystem.h"
#include "matrix.h"
#include "lilyparser.h"
#include "hashtable.h"
#include "assetloader.h"
#include "preprocess.h"

// Various useful #defines...
#if !defined(MIN) || !defined(MAX)
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#if !defined(OFFSET_OF)
// Get the offset if a field in a structure, in bytes. Does an ugly
// hack to get around compiler complaining about possibly
// dereferencing something from NULL.
#define OFFSET_OF(t, x) ((char*)&(((t*)(0x1))->x) - 1)
#endif




