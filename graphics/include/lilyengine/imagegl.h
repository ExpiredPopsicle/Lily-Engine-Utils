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
            bool compressedTexture = true);

    }
}
