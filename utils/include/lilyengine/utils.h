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

#pragma once

#include "streams/streamsection.h"
#include "streams/owningstream.h"

#include "deflate/deflate.h"
#include "deflate/deflate_streambuf.h"
#include "deflate/deflate_zlib.h"
#include "deflate/zipfile.h"

#include "image.h"
#include "config.h"
#include "winhacks.h"
#include "malstring.h"
#include "base64.h"
#include "filesystem.h"
#include "matrix.h"
#include "angle.h"
#include "lilyparser.h"
#include "lilyparserxml.h"
#include "lilyparserjson.h"
#include "assetloader.h"
#include "preprocess.h"
#include "cellarray.h"
#include "expopsockets.h"
#include "params.h"
#include "params_advanced.h"
#include "http.h"
#include "cryptorc4.h"
#include "compress.h"
#include "archive.h"
#include "assetmanager.h"

#include "graphicalconsole/graphicalconsole.h"

#include "pixelimage/pixelimage.h"
#include "pixelimage/pixelimage_legacy.h"
#include "pixelimage/pixelimage_scale.h"
#include "pixelimage/pixelimage_tga.h"
#include "pixelimage/pixelimage_blit.h"
#include "pixelimage/pixelimage_stb.h"

// TODO: Move these to a math module or something.
namespace ExPop
{
    template<typename T>
    inline const T &min(const T &a, const T &b)
    {
        return (b < a) ? b : a;
    }

    template<typename T>
    inline const T &max(const T &a, const T &b)
    {
        return (b > a) ? b : a;
    }
}


