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

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    // STB header must be included before this header for this part to work.
  #ifdef STBI_INCLUDE_STB_IMAGE_H

    PixelImage<uint8_t> *pixelImageLoadSTB(const std::string &data);
    PixelImage<uint8_t> *pixelImageLoadSTBFromFile(const std::string &filename);

  #endif
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
  #ifdef STBI_INCLUDE_STB_IMAGE_H

    PixelImage<uint8_t> *pixelImageLoadSTB(const std::string &data)
    {
        int width = 0;
        int height = 0;
        int channelCount = 0;

        stbi_uc *imgData = stbi_load_from_memory(
            (stbi_uc*)&data[0], data.size(), &width, &height, &channelCount, 0);

        if(!imgData) {
            return nullptr;
        }

        PixelImage<uint8_t> *newImage = new PixelImage<uint8_t>(width, height, channelCount);

        stbi_uc *ptr = imgData;

        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                for(int c = 0; c < channelCount; c++) {
                    newImage->getData(x, y, c).value = *ptr;
                    ptr++;
                }
            }
        }

        STBI_FREE(imgData);

        return newImage;
    }

    PixelImage<uint8_t> *pixelImageLoadSTBFromFile(const std::string &filename)
    {
        std::string fileData = FileSystem::loadFileString(filename);
        if(fileData.size()) {
            return pixelImageLoadSTB(fileData);
        }
        return nullptr;
    }

  #endif
}


