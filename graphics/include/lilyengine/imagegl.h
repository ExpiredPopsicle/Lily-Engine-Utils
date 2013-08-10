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

namespace ExPop {
    namespace Gfx {

        class Image;

        /// Output from the creation of an OpenGL texture from an
        /// Image object.
        struct ImageTexture {

            /// The OpenGL texture number.
            unsigned int glTexNum;

            /// The width and height if the image needed to be resized
            /// to match a power of two size.
            unsigned int pow2Width;
            unsigned int pow2Height;

            /// The original width and height.
            unsigned int origWidth;
            unsigned int origHeight;
        };

        enum ImageFormat {
            IMAGE_FORMAT_RGBA_32,
            IMAGE_FORMAT_GREY_8
        };

        /// Create an OpenGL texture from an Image. imgTex gets filled
        /// with information about the texture, including the
        /// generated GL texture number.
        void createGLTextureFromImage(
            Image *image,
            ImageTexture *imgTex,
            ImageFormat format = IMAGE_FORMAT_RGBA_32,
            bool compressedTexture = true,
            bool convertPow2 = true);

    }
}
