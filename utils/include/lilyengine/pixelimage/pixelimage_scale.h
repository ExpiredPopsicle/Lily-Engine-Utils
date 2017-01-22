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

// Function definitions for image scaling operations.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "pixelvalue.h"
#include "pixelimagebase.h"
#include "pixelimage.h"

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    template<typename ValueType, ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    PixelImage<ValueType, scalingType> *pixelImageScale(
        PixelImageBase &inputImage,
        PixelImage_Dimension width,
        PixelImage_Dimension height);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    template<typename ValueType, ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    inline PixelImage<ValueType, scalingType> *upScaleImageLinear(
        const PixelImageBase &inputImage,
        int width, int height,
        PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Clamp)
    {
        PixelImage<ValueType, scalingType> *outputImage =
            new PixelImage<ValueType, scalingType>(
                width, height, inputImage.getChannelCount());

        float dstPixelSize_x = 1.0f / float(width);
        float dstPixelSize_y = 1.0f / float(height);

        for(PixelImage_Coordinate y = 0; y < height; y++) {
            for(PixelImage_Coordinate x = 0; x < width; x++) {
                for(PixelImage_Coordinate c = 0; c < inputImage.getChannelCount(); c++) {

                    // Create normalized coordinates.
                    float nx = float(x) * dstPixelSize_x + dstPixelSize_x * 0.5f;
                    float ny = float(y) * dstPixelSize_y + dstPixelSize_y * 0.5f;

                    double p = inputImage.sampleWithHalfPixelOffset(
                        nx,
                        ny,
                        c,
                        edgeMode);

                    outputImage->setDouble(x, y, c, p);
                }
            }
        }

        return outputImage;
    }

    inline double getRowSectionAverage(
        const PixelImageBase &img,
        PixelImage_Coordinate row,
        float colStart,
        float colEnd,
        PixelImage_Coordinate channel)
    {
        // First of all, the dumb case. Are they both in the same pixel?
        if(floor(colStart) == floor(colEnd)) {
            float a0 = 1.0f - (colStart - floor(colStart));
            float a1 = 1.0f - (floor(colEnd) + 1.0f - colEnd);
            return (
                img.getDouble(int(colStart), row, channel) * a0 +
                img.getDouble(int(colEnd)+1, row, channel) * a1) * (1.0f / (a0 + a1));
        }

        // Handle the first fractional pixel and determine the actual
        // starting point.
        int realStart = 0; // Index of the first non-fractional (whole) pixel.
        float startAlpha = 0.0f; // Alpha value of the starting fractional pixel.
        double startPixel = 0.0; // Value of the starting fractional pixel.

        if(int(colStart) == colStart) {

            // Starts on a pixel boundary. Don't worry about fractional
            // pixels.
            realStart = floor(colStart);
            startAlpha = 0.0f;

        } else {

            realStart = int(floor(colStart)) + 1; // Whole pixels start after the first one.

            // startAlpha = 1.0f - (colStart - realStart);
            startAlpha = 1.0f - (float(realStart) - colStart);

            startPixel = img.getDouble(realStart - 1, row, channel);

        }

        // Handle last fractional pixel.
        int realEnd = 0;
        float endAlpha = 0.0f;
        double endPixel = 0.0f;

        if(int(colEnd) == colEnd) {

            // Ends on a pixel boundary. Don't worry about fractional
            // pixels.
            realEnd = int(colEnd);
            endAlpha = 0.0f;

        } else {

            realEnd = int(floor(colEnd)) - 1; // Whole pixels end right before this one.
            endAlpha = colEnd - realEnd; // frac(), basically.

            endPixel = img.getDouble(realEnd + 1, row, channel);

        }

        // Add up all the pixels in between.
        double outputPixel = 0.0;
        for(int i = realStart; i <= realEnd; i++) {
            outputPixel = outputPixel + img.getDouble(i, row, channel);
        }
        outputPixel = outputPixel + startPixel * startAlpha;
        outputPixel = outputPixel + endPixel * endAlpha;

        // Now divide by the total alpha.
        outputPixel = outputPixel * (1.0f / float(realEnd - realStart + 1 + startAlpha + endAlpha));

        return outputPixel;
    }



    inline double getSectionAverage(
        const PixelImageBase &img,
        float rowStart,
        float rowEnd,
        float colStart,
        float colEnd,
        PixelImage_Coordinate channel)
    {
        // First of all, the dumb case. Are they both in the same pixel?
        if(int(rowStart) == int(rowEnd)) {

            float a0 = 1.0f - (rowStart - floor(rowStart));
            float a1 = 1.0f - (floor(rowEnd) + 1.0f - rowEnd);
            return (
                getRowSectionAverage(
                    img, PixelImage_Coordinate(rowStart), colStart, colEnd, channel) * a0 +
                getRowSectionAverage(
                    img, PixelImage_Coordinate(rowEnd) + 1, colStart, colEnd, channel) * a1) * (1.0f / (a0 + a1));
        }

        // Handle the first fractional row and determine the actual
        // starting point.
        int realStart = 0;
        float startAlpha = 0.0f;
        double startRow = 0.0;

        if(int(rowStart) == rowStart) {

            // Starts on a row boundary. Don't worry about fractional
            // rows.
            realStart = floor(rowStart);
            startAlpha = 0.0f;

        } else {

            realStart = int(floor(rowStart)) + 1;

            // startAlpha = 1.0f - (rowStart - realStart);
            startAlpha = 1.0f - (float(realStart - rowStart));

            startRow = getRowSectionAverage(
                img, realStart - 1, colStart, colEnd, channel);

        }

        // Handle last fractional pixel.
        int realEnd = 0;
        float endAlpha = 0.0f;
        double endRow = 0.0;

        if(int(rowEnd) == rowEnd) {

            // Ends on a row boundary. Don't worry about fractional rows.
            realEnd = int(rowEnd);
            endAlpha = 0.0f;

        } else {

            realEnd = int(floor(rowEnd)) - 1;
            endAlpha = rowEnd - realEnd;

            endRow = getRowSectionAverage(
                img, realEnd + 1, colStart, colEnd, channel);
        }

        // Add up all the pixels in between.
        double outputPixel = 0.0;
        for(int i = realStart; i <= realEnd; i++) {
            outputPixel = outputPixel + getRowSectionAverage(
                img, i, colStart, colEnd, channel);
        }
        outputPixel = outputPixel + startRow * startAlpha;
        outputPixel = outputPixel + endRow * endAlpha;

        // Now divide by the total alpha.
        float alphaInverse = float(realEnd - realStart + 1 + startAlpha + endAlpha);
        float alpha = 1.0f / alphaInverse;
        outputPixel = outputPixel * alpha;

        return outputPixel;
    }

    template<typename ValueType, ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    inline PixelImage<ValueType, scalingType> *downScaleImageAveraged(
        PixelImageBase &inputImage,
        PixelImage_Dimension width,
        PixelImage_Dimension height)
    {
        PixelImage<ValueType, scalingType> *outputImage =
            new PixelImage<ValueType, scalingType>(
                width, height, inputImage.getChannelCount());

        float xstep = float(inputImage.getWidth()) / float(width);
        float ystep = float(inputImage.getHeight()) / float(height);

        for(PixelImage_Coordinate y = 0; y < PixelImage_Coordinate(height); y++) {
            for(PixelImage_Coordinate x = 0; x < PixelImage_Coordinate(width); x++) {
                for(PixelImage_Coordinate channel = 0; channel < PixelImage_Coordinate(inputImage.getChannelCount()); channel++) {
                    outputImage->setDouble(
                        x, y,
                        channel,
                        getSectionAverage(
                            inputImage,
                            y * ystep, (y + 1) * ystep - 1,
                            x * xstep, (x + 1) * xstep - 1,
                            channel));
                }
            }
        }

        return outputImage;
    }

    template<typename ValueType, ScalingType scalingType>
    inline PixelImage<ValueType, scalingType> *pixelImageScale(
        PixelImageBase &inputImage,
        PixelImage_Dimension width,
        PixelImage_Dimension height)
    {
        if(width <= 1 || height <= 1) {
            return nullptr;
        }

        PixelImage_Dimension origWidth = inputImage.getWidth();
        PixelImage_Dimension origHeight = inputImage.getHeight();

        PixelImage<ValueType, scalingType> *ret = nullptr;

        if(width == origWidth && height == origHeight) {

            // Size is identical. Just return a snapshot.
            ret = new PixelImage<ValueType, scalingType>(inputImage);

        } else if(width > origWidth && height > origHeight) {

            // Scale up everything.
            ret = upScaleImageLinear<ValueType, scalingType>(inputImage, width, height);

        } else if(width < origWidth && height < origHeight) {

            // Scale down everything.
            ret = downScaleImageAveraged<ValueType, scalingType>(inputImage, width, height);

        } else if(width > origWidth && height <= origHeight) {

            // Scale up on x axis and scale down on y axis.
            PixelImage<ValueType, scalingType> *tmp =
                upScaleImageLinear<ValueType, scalingType>(inputImage, width, origHeight);
            PixelImage<ValueType, scalingType> *out =
                downScaleImageAveraged<ValueType, scalingType>(*tmp, width, height);

            delete tmp;
            ret = out;

        } else if(width <= origWidth && height > origHeight) {

            // Scale up on y axis and scale down on x axis.
            PixelImage<ValueType, scalingType> *tmp =
                upScaleImageLinear<ValueType, scalingType>(inputImage, origWidth, height);
            PixelImage<ValueType, scalingType> *out =
                downScaleImageAveraged<ValueType, scalingType>(*tmp, width, height);

            delete tmp;
            ret = out;

        } else {

            // FIXME: Can we reach this?
            ret = nullptr;
        }

        return ret;
    }
}

