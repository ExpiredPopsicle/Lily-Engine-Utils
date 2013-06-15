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

#include <iostream>
using namespace std;

#include <GL/gl.h>

#include <lilyengine/console.h>
#include <lilyengine/image.h>
using namespace ExPop::Console;

#include <lilyengine/glcontext.h>
#include <lilyengine/imagegl.h>

namespace ExPop {

    namespace Gfx {

        void createGLTextureFromImage(
            Image *img,
            ImageTexture *imgTex,
            ImageFormat format,
            bool compressedTexture,
            bool convertPow2) {

            GLuint texNum;
            glGenTextures(1, &texNum);

            glBindTexture(GL_TEXTURE_2D, texNum);

            EXPOP_ASSERT_GL();

            imgTex->glTexNum = texNum;

            unsigned int width = img->getWidth();
            unsigned int height = img->getHeight();

            if(convertPow2) {

                imgTex->pow2Width = 1;
                while(imgTex->pow2Width < width) {
                    imgTex->pow2Width <<= 1;
                }

                imgTex->pow2Height = 1;
                while(imgTex->pow2Height < height) {
                    imgTex->pow2Height <<= 1;
                }

            } else {

                imgTex->pow2Width = width;
                imgTex->pow2Height = height;

            }

            imgTex->origWidth = width;
            imgTex->origHeight = height;

            Image *adjustedImage = img;
            Image *resizedImage = NULL;
            unsigned char *convertedImage = NULL;

            if(imgTex->origHeight != imgTex->pow2Height || imgTex->origWidth != imgTex->pow2Width) {

                // The image isn't a power of two size. Needs to be reprocessed.
                out("warning") << "Processing non-power-of-two texture! This is slow!" << endl;

                resizedImage = new Image(imgTex->pow2Width, imgTex->pow2Height);

                for(unsigned int x = 0; x < imgTex->pow2Width; x++) {
                    for(unsigned int y = 0; y < imgTex->pow2Height; y++) {
                        if(x < (unsigned int)width && y < (unsigned int)height) {
                            const Pixel *src = img->getPixel(x, y);
                            Pixel p;
                            p.colorsAsArray[3] = src->rgba.a;
                            p.colorsAsArray[1] = src->rgba.g;

                            p.colorsAsArray[0] = src->rgba.r;
                            p.colorsAsArray[2] = src->rgba.b;

                            resizedImage->setPixel(p, x, y);
                        } else {
                            Pixel black = {0};
                            resizedImage->setPixel(black, x, y);
                        }
                    }
                }

                adjustedImage = resizedImage;
            }

            if(format != IMAGE_FORMAT_RGBA_32) {

                switch(format) {

                    case IMAGE_FORMAT_RGBA_32:
                        // GCC complains if I don't have this here. Derp.
                        break;

                    case IMAGE_FORMAT_GREY_8:

                        convertedImage =
                            new unsigned char[imgTex->pow2Height * imgTex->pow2Width];

                        for(unsigned int x = 0; x < imgTex->pow2Width; x++) {
                            for(unsigned int y = 0; y < imgTex->pow2Height; y++) {
                                unsigned char *outChar = &(convertedImage[x + imgTex->pow2Width * y]);

                                // We're just doing an average here
                                // instead of any kind of proper
                                // luminance conversion.
                                Pixel *p = adjustedImage->getPixel(x, y);
                                *outChar = (p->rgba.r + p->rgba.g + p->rgba.b) / 3;
                            }
                        }

                        break;
                }

            }

            unsigned int realFormat = GL_RGBA;
            unsigned int realInternal = compressedTexture ? GL_COMPRESSED_RGBA : GL_RGBA;

            if(convertedImage) {

                switch(format) {
                    case IMAGE_FORMAT_RGBA_32:
                        break;
                    case IMAGE_FORMAT_GREY_8:
                        realFormat = GL_LUMINANCE;
                        realInternal = compressedTexture ? GL_COMPRESSED_LUMINANCE : GL_LUMINANCE;
                        break;
                    default:
                        break;
                }

                // FIXME: Endian issues here, probably.
                glTexImage2D(
                    GL_TEXTURE_2D, 0,
                    realInternal, imgTex->pow2Width, imgTex->pow2Height, 0,
                    realFormat, GL_UNSIGNED_BYTE, convertedImage);

            } else {

                // FIXME: Endian issues here, probably.
                glTexImage2D(
                    GL_TEXTURE_2D, 0,
                    realInternal, imgTex->pow2Width, imgTex->pow2Height, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, adjustedImage->getPixel(0, 0));

            }

            if(resizedImage) {
                delete resizedImage;
            }

            if(convertedImage) {
                delete[] convertedImage;
            }

            EXPOP_ASSERT_GL();

            // Set up really basic min/mag filters.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            EXPOP_ASSERT_GL();

        }

    }

}


