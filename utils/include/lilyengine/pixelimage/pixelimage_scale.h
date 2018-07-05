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
    inline PixelImage<ValueType, scalingType> *pixelImageScale_old(
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




    // const int pixelImage_sinTableSize = 65536;
    // const float pixelImage_2pi = 3.14159f * 2.0f;

    // inline float pixelImage_sinTableIndexToValue(int x)
    // {
    //     return
    //         pixelImage_2pi *
    //         float(x - pixelImage_sinTableSize / 2) /
    //         float(pixelImage_sinTableSize / 2);
    // }

    // inline float *pixelImage_genSinTable()
    // {
    //     static float ret[pixelImage_sinTableSize];
    //     for(size_t i = 0; i < pixelImage_sinTableSize; i++) {
    //         ret[i] = sin(pixelImage_sinTableIndexToValue(i));
    //     }
    //     return ret;
    // }

    // inline float pixelImage_sinTable(int x)
    // {
    //     static float *table =
    //         pixelImage_genSinTable();
    //     return table[x];
    // }

    // inline int pixelImage_sinTableValueToIndex(float x)
    // {
    //     // return (pixelImage_sinTableSize / 2) +
    //     //     int(float(pixelImage_sinTableSize / 2) *
    //     //         fmod(x, pixelImage_2pi) /
    //     //         pixelImage_2pi);

    //     // int n = (pixelImage_sinTableSize / 2) +
    //     //     int(float(pixelImage_sinTableSize / 2) *
    //     //         fmod(x, pixelImage_2pi) /
    //     //         pixelImage_2pi);

    //     int n =
    //         (pixelImage_sinTableSize / 2) +
    //         int(float(pixelImage_sinTableSize / 2) *
    //             x /
    //             pixelImage_2pi) % (pixelImage_sinTableSize / 2);

    //     // assert(n >= 0 && n <= pixelImage_sinTableSize);
    //     // std::cout << n << " " << j << std::endl;

    //     return n;
    // }

    // inline float pixelImage_sinTableLookup(float x)
    // {
    //     int m = pixelImage_sinTableValueToIndex(x);
    //     return pixelImage_sinTable(m);
    // }

    inline float pixelImage_lanczosSinc(float x)
    {
        // return pixelImage_sinTableLookup(x) / x;
        return sin(x) / x;
    }

    inline float pixelImage_lanczosFilter(float x, float a)
    {
        if(x == 0.0f) {
            return 1.0f;
        } else if(x >= -a && x <= a) {
            return
                pixelImage_lanczosSinc(3.14159f * x) *
                pixelImage_lanczosSinc(3.14159f * x/a);
        }
        return 0.0f;
    }

    template<typename ValueType, ScalingType scalingType>
    inline PixelImage<ValueType, scalingType> *pixelImageScale_lanczos(
        PixelImageBase &inputImage,
        PixelImage_Dimension width,
        PixelImage_Dimension height)
    {
        if(width <= 1 || height <= 1) {
            return nullptr;
        }

        // DONOTCHECKIN TODO - fix naming.
        PixelImage<ValueType, scalingType> *testThing =
            new PixelImage<ValueType, scalingType>(
                width, height, inputImage.getChannelCount());

        const float a = 3.0f;

        for(PixelImage_Coordinate y = 0; y < testThing->getHeight(); y++) {
            for(PixelImage_Coordinate x = 0; x < testThing->getWidth(); x++) {
                for(PixelImage_Coordinate c = 0; c < testThing->getChannelCount(); c++) {

                    float sx = (float(x) / float(testThing->getWidth())) * float(inputImage.getWidth());
                    float sy = (float(y) / float(testThing->getHeight())) * float(inputImage.getHeight());
                    // testThing->getData(x, y, c).setScaledValue(inputImage.getDouble(sx, sy, c));

                    float sourceStartX = floor(sx) - a + 1;
                    float sourceEndX = floor(sx) + a;
                    float sourceStartY = floor(sy) - a + 1;
                    float sourceEndY = floor(sy) + a;
                    float currentVal = 0.0f;
                    float maxVal = 0.0f;

                    for(float sourceY = sourceStartY; sourceY <= sourceEndY; sourceY++) {
                        for(float sourceX = sourceStartX; sourceX <= sourceEndX; sourceX++) {

                            // // TODO: Remove this.
                            // float xval = pixelImage_lanczosFilter(sx - sourceX, a);
                            // float yval = pixelImage_lanczosFilter(sy - sourceY, a);

                            float lval =
                                pixelImage_lanczosFilter(sx - sourceX, a) *
                                pixelImage_lanczosFilter(sy - sourceY, a);

                            // if(y == 91) {
                            //     std::cout << "  lval: " << lval << "=" << xval << "*" << yval << std::endl;
                            //     std::cout << "  x: " << sx - sourceX << std::endl;
                            //     std::cout << "  y: " << sy - sourceY << std::endl;
                            // }

                            maxVal += lval;
                            currentVal += inputImage.getDouble(sourceX, sourceY, c) * lval;
                        }
                    }
                    // std::cout << y << " " << maxVal << std::endl;
                    testThing->getData(x, y, c).setScaledValue(currentVal / maxVal);
                }
            }
        }

        return testThing;
    }


    template<typename ValueType, ExPop::ScalingType scalingType>
    inline ExPop::PixelImage<ValueType, scalingType> *pixelImageHalfRes(
        ExPop::PixelImageBase &inputImage,
        bool axis)
    {
        ExPop::PixelImage_Dimension dims[2] = {
            inputImage.getWidth(),
            inputImage.getHeight()
        };

        ExPop::PixelImage_Dimension newDims[2] = {
            axis == false ? (dims[0] / 2) : dims[0],
            axis == true  ? (dims[1] / 2) : dims[1]
        };

        ExPop::PixelImage<ValueType, scalingType> *ret = new ExPop::PixelImage<ValueType, scalingType>(
            newDims[0], newDims[1], inputImage.getChannelCount());

        for(ExPop::PixelImage_Coordinate c = 0; c < inputImage.getChannelCount(); c++) {
            for(ExPop::PixelImage_Coordinate keepAxis = 0; keepAxis < newDims[!axis]; keepAxis++) {
                for(ExPop::PixelImage_Coordinate changeAxis = 0; changeAxis < newDims[axis]; changeAxis++) {

                    ExPop::PixelImage_Coordinate pos[2] = {
                        axis == false ? changeAxis : keepAxis,
                        axis == true  ? changeAxis : keepAxis
                    };
                    ExPop::PixelImage_Coordinate srcPos1[2] = {
                        axis == false ? changeAxis * 2 : keepAxis,
                        axis == true  ? changeAxis * 2: keepAxis
                    };
                    ExPop::PixelImage_Coordinate srcPos2[2] = {
                        axis == false ? changeAxis * 2 + 1: keepAxis,
                        axis == true  ? changeAxis * 2 + 1: keepAxis
                    };

                    double avg =
                        (inputImage.getDouble(srcPos1[0], srcPos1[1], c) +
                            inputImage.getDouble(srcPos1[0], srcPos2[1], c)) / 2.0f;
                    ret->setDouble(pos[0], pos[1], c, avg);
                }
            }
        }

        return ret;
    }

    template<typename ValueType, ScalingType scalingType>
    inline PixelImage<ValueType, scalingType> *pixelImageScale(
        PixelImageBase &inputImage,
        PixelImage_Dimension width,
        PixelImage_Dimension height)
    {
        ExPop::PixelImage_Dimension hrWidth = inputImage.getWidth();
        ExPop::PixelImage_Dimension hrHeight = inputImage.getHeight();

        std::cout << "sdaf1" << std::endl;

        // Determine how far we can go by half-rezzing.
        if(width < inputImage.getWidth()) {
            while((hrWidth >> 1) >= width) {
                hrWidth >>= 1;
            }
            while((hrHeight >> 1) >= height) {
                hrHeight >>= 1;
            }
        }

        std::cout << "sdaf2" << std::endl;

        ExPop::PixelImage<double> *tmp = new ExPop::PixelImage<double>(inputImage);;
        while(tmp->getWidth() > hrWidth) {
            ExPop::PixelImage<double> *tmp2 = nullptr;
            // if(tmp->getWidth() & 1) {
            //     // std::cout << "Lanc1" << std::endl;
            //     tmp2 = pixelImageScale_lanczos<double, ExPop::ScalingType_OneIsOne>(
            //         *tmp, tmp->getWidth() / 2, tmp->getHeight());
            // } else {
                tmp2 = pixelImageHalfRes<double, ExPop::ScalingType_OneIsOne>(*tmp, false);
            // }
            delete tmp;
            tmp = tmp2;
        }
        assert(tmp->getWidth() == hrWidth);

        std::cout << "sdaf3" << std::endl;

        while(tmp->getHeight() > hrHeight) {
            ExPop::PixelImage<double> *tmp2 = nullptr;
            // // if(tmp->getHeight() & 1) {
            //     std::cout << "Lanc2" << std::endl;
                // tmp2 = pixelImageScale_lanczos<double, ExPop::ScalingType_OneIsOne>(
                //     *tmp, tmp->getWidth(), tmp->getHeight() / 2);
            // } else {
                tmp2 = pixelImageHalfRes<double, ExPop::ScalingType_OneIsOne>(*tmp, true);
            // }
            delete tmp;
            tmp = tmp2;
        }
        assert(tmp->getHeight() == hrHeight);

        std::cout << "sdaf4" << std::endl;

        ExPop::PixelImage<uint8_t> *scaled3 =
            ExPop::pixelImageScale_lanczos<ValueType, scalingType>(*tmp, width, height);
            // new ExPop::PixelImage<uint8_t>(*tmp);

        std::cout << "sdaf5" << std::endl;

        delete tmp;

        return scaled3;
    }

}

