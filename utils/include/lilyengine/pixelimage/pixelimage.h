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

// Arbitrary-format image class.

// This is meant to be a replacement for ExPop::Gfx::Image, which is
// old, icky, and only supports 32-bit RGBA.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include "pixelvalue.h"
#include "pixelimagebase.h"

#include <cstring>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Image type that actually has a format.
    template<
        typename ValueType,
        ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    class PixelImage : public PixelImageBase
    {
    public:

        /// Internal pixel data type.
        typedef PixelValue<ValueType, scalingType> PixelValueType;

        /// Our own type.
        typedef PixelImage<ValueType, scalingType> MyType;

        // ----------------------------------------------------------------------
        // Constructors, destructor, and copying stuff.

        /// Default constructor. Creates a 1x1 pixel image with 1
        /// channel and uninitialized data.
        PixelImage();

        /// Copy constructor for same-type.
        PixelImage(const MyType &other);

        /// Copy constructor for unknown type. (Arbitrary
        /// PixelImageBase that may be a different internal format.)
        PixelImage(const PixelImageBase &other);

        /// Constructor with width/height/channels.
        PixelImage(
            PixelImage_Dimension inWidth,
            PixelImage_Dimension inHeight,
            PixelImage_Dimension inNumChannels);

        /// Copy another image of arbitrary type into this with
        /// operator=.
        MyType &operator=(const PixelImageBase &other);

        /// Copy another image of THIS type into this with operator=.
        MyType &operator=(const MyType &other);

        /// Destructor.
        virtual ~PixelImage();

        // ----------------------------------------------------------------------
        // Pixel data access and modification.

        /// Get a single channel from a single pixel.
        PixelValueType &getData(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap);

        const PixelValueType &getData(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const;

        // ----------------------------------------------------------------------
        // PixelImageBase interface implementation.

        double getDouble(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const override;

        void setDouble(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            double value,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) override;

        void setSizeAndChannels(
            PixelImage_Dimension inWidth,
            PixelImage_Dimension inHeight,
            PixelImage_Dimension inNumChannels) override;

        void *getRawData() override;
        size_t getRawDataLength() const override;

    private:

        PixelValueType *data;
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    // Basic constructor
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType>::PixelImage()
    {
        data = new PixelValueType[1];
        width = 1;
        height = 1;
        numChannels = 1;

        // Clear to black.
        data->template setScaledValue<float>(0.0f);
    }

    // Copy constructor
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType>::PixelImage(const MyType &other)
    {
        width = other.width;
        height = other.height;
        numChannels = other.numChannels;

        data = new PixelValueType[width * height * numChannels];

        size_t dataSize = width * height * numChannels * sizeof(PixelValueType);
        memcpy(data, other.data, dataSize);
    }

    // Copy constructor from unknown type.
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType>::PixelImage(const PixelImageBase &other)
    {
        width = other.getWidth();
        height = other.getHeight();
        numChannels = other.getChannelCount();

        data = new PixelValueType[width * height * numChannels];

        // Copy data over, rescaling as needed.
        for(PixelImage_Coordinate y = 0; y < height; y++) {
            for(PixelImage_Coordinate x = 0; x < width; x++) {
                for(PixelImage_Coordinate c = 0; c < numChannels; c++) {
                    setDouble(x, y, c, other.getDouble(x, y, c));
                }
            }
        }
    }

    // Constructor with dimensions.
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType>::PixelImage(
        PixelImage_Dimension inWidth,
        PixelImage_Dimension inHeight,
        PixelImage_Dimension inNumChannels)
    {
        if(inWidth < 1) inWidth = 1;
        if(inHeight < 1) inHeight = 1;
        if(inNumChannels < 1) inNumChannels = 1;

        width = inWidth;
        height = inHeight;
        numChannels = inNumChannels;
        data = new PixelValueType[width * height * numChannels];

        memset((void*)data, 0, sizeof(PixelValueType) * width * height * numChannels);
    }

    // Destructor
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType>::~PixelImage()
    {
        delete[] data;
        data = nullptr;
    }

    // getData
    template<
        typename ValueType,
        ScalingType scalingType>
    inline typename PixelImage<ValueType, scalingType>::PixelValueType &PixelImage<ValueType, scalingType>::getData(
        PixelImage_Coordinate x,
        PixelImage_Coordinate y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode)
    {
        size_t index = findIndex(x, y, channel, edgeMode);
        return data[index];
    }

    // getData (const)
    template<
        typename ValueType,
        ScalingType scalingType>
    inline const typename PixelImage<ValueType, scalingType>::PixelValueType &PixelImage<ValueType, scalingType>::getData(
        PixelImage_Coordinate x,
        PixelImage_Coordinate y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        size_t index = findIndex(x, y, channel, edgeMode);
        return data[index];
    }

    // setSizeAndChannels
    template<
        typename ValueType,
        ScalingType scalingType>
    inline void PixelImage<ValueType, scalingType>::setSizeAndChannels(
        PixelImage_Dimension inWidth,
        PixelImage_Dimension inHeight,
        PixelImage_Dimension inNumChannels)
    {
        // Make a new image buffer and clear it.
        PixelValueType *newData = new PixelValueType[inWidth * inHeight * inNumChannels];
        memset((void*)newData, 0, sizeof(PixelValueType) * inWidth * inHeight * inNumChannels);

        // Copy old image over.
        for(PixelImage_Coordinate y = 0; y < height && y < inHeight; y++) {
            for(PixelImage_Coordinate x = 0; x < width && x < inWidth; x++) {
                for(PixelImage_Coordinate c = 0; c < numChannels && c < inNumChannels; c++) {
                    newData[c + x * inNumChannels + y * (inWidth * inNumChannels)].value =
                        data[c + x * numChannels + y * (width * numChannels)].value;
                }
            }
        }

        // Swap buffers around.
        delete[] data;
        data = newData;
        width = inWidth;
        height = inHeight;
        numChannels = inNumChannels;
    }

    // getDouble
    template<
        typename ValueType,
        ScalingType scalingType>
    inline double PixelImage<ValueType, scalingType>::getDouble(
        PixelImage_Coordinate x,
        PixelImage_Coordinate y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        // FIXME: Go through setPixelChannel so we funnel everything
        // through the same point.
        size_t index = findIndex(x, y, channel, edgeMode);
        return data[index].template getScaledValue<double>();
    }

    // setDouble
    template<
        typename ValueType,
        ScalingType scalingType>
    inline void PixelImage<ValueType, scalingType>::setDouble(
        PixelImage_Coordinate x,
        PixelImage_Coordinate y,
        PixelImage_Coordinate channel,
        double value,
        PixelImage_EdgeMode edgeMode)
    {
        // FIXME: Go through setPixelChannel so we funnel everything
        // through the same point.
        size_t index = findIndex(x, y, channel, edgeMode);
        data[index].template setScaledValue<double>(value);
    }

    // operator= (From other generic image)
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType> &PixelImage<ValueType, scalingType>::operator=(
        const PixelImageBase &other)
    {
        // Just redirect this to the base class's version.
        PixelImageBase::operator=(other);
        return *this;
    }

    // operator= (From other image of same type)
    template<
        typename ValueType,
        ScalingType scalingType>
    inline PixelImage<ValueType, scalingType> &PixelImage<ValueType, scalingType>::operator=(
        const PixelImage<ValueType, scalingType> &other)
    {
        delete[] data;

        width = other.width;
        height = other.height;
        numChannels = other.numChannels;

        data = new PixelValueType[width * height * numChannels];

        size_t dataSize = width * height * numChannels * sizeof(PixelValueType);
        memcpy(data, other.data, dataSize);

        return *this;
    }

    // getRawData
    template<
        typename ValueType,
        ScalingType scalingType>
    void *PixelImage<ValueType, scalingType>::getRawData()
    {
        return (void*)data;
    }

    // getRawDataLength
    template<
        typename ValueType,
        ScalingType scalingType>
    size_t PixelImage<ValueType, scalingType>::getRawDataLength() const
    {
        return sizeof(PixelValueType) * width * height * numChannels;
    }
}

