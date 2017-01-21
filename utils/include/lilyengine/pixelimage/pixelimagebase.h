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

// Base class for the templated arbitrary-format image class.

// A lot of the time we don't actually care (much) what the udnerlying
// data format of an image is. This base class should have all the
// format-agnostic functionality we need, and can still get/set data,
// but has to use generic scaled (float) values to do so.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Wrapping modes for some operations.
    enum PixelImage_EdgeMode
    {
        PixelImage_EdgeMode_Wrap,
        PixelImage_EdgeMode_Clamp
    };

    // TODO: Min/Mag filters?

    typedef int32_t PixelImage_Coordinate;
    typedef uint32_t PixelImage_Dimension;

    /// Base image type abstract class. Format-agnostic.
    class PixelImageBase
    {
    public:

        // ----------------------------------------------------------------------
        // Base class constructor/destructor and copy stuff.

        PixelImageBase();
        virtual ~PixelImageBase();

        /// This operator=() will copy with a format converion,
        /// regardless of what type comes into it (even if both
        /// operands are actually the same internal type). It can be
        /// okay when we don't have visibility of the original types
        /// anymore, but should be a last resort, or used when we
        /// don't care about precision/speed loss.
        PixelImageBase &operator=(const PixelImageBase &other);

        // ----------------------------------------------------------------------
        // Size and channel count access and modification.

        /// Resize the image. Original data will be anchored the
        /// upper-left corner. Existing channels greater than the new
        /// channel count will be dropped. New channels and area will
        /// be cleared to zero.
        virtual void setSizeAndChannels(
            PixelImage_Dimension inWidth,
            PixelImage_Dimension inHeight,
            PixelImage_Dimension inNumChannels) = 0;

        /// Convenience function to set the size without altering the
        /// channel count.
        void setSize(
            PixelImage_Dimension inWidth,
            PixelImage_Dimension inHeight);

        /// Convenience function to set the channel count without altering
        /// the size.
        void setNumChannels(
            PixelImage_Dimension inNumChannels);

        /// Get the width.
        PixelImage_Dimension getWidth() const;

        /// Get the height.
        PixelImage_Dimension getHeight() const;

        /// Get the number of channels.
        PixelImage_Dimension getChannelCount() const;

        // TODO: scale(). (Pull code from Compound to do this.)

        // ----------------------------------------------------------------------
        // Pixel data access and modification.

        virtual void *getRawData() = 0;
        virtual size_t getRawDataLength() const = 0;

        virtual double getDouble(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const = 0;

        virtual void setDouble(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate channel,
            double value,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) = 0;

        double sample(
            float x, float y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const;

        /// In case you wanted to re-live horrible memories of D3D9's
        /// half-texel offset coordinates....
        double sampleWithHalfPixelOffset(
            float x, float y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const;

        double getInterpolatedValue(
            float x, float y,
            PixelImage_Coordinate channel,
            PixelImage_EdgeMode edgeMode = PixelImage_EdgeMode_Wrap) const;

        // ----------------------------------------------------------------------
        // Format conversion and serialization.

        // TODO: Load/save PNG? (libpng? stb_image?)

        // TODO: Load/save stb_image in a generic way?

        // TODO: Load/save native format. (Load will have to be some
        //   kind of static factory, because of types dependent on
        //   saved format. Save will have to be virtual, with specific
        //   format stuff.)

    protected:

        PixelImage_Dimension width;
        PixelImage_Dimension height;
        PixelImage_Dimension numChannels;

        /// Find the actual index for some coordinates.
        size_t findIndex(
            PixelImage_Coordinate x,
            PixelImage_Coordinate y,
            PixelImage_Coordinate inChannel,
            PixelImage_EdgeMode edgeMode // = PixelImage_EdgeMode_Wrap
            ) const;
    };
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline void PixelImageBase::setSize(
        PixelImage_Dimension inWidth,
        PixelImage_Dimension inHeight)
    {
        setSizeAndChannels(inWidth, inHeight, numChannels);
    }

    inline void PixelImageBase::setNumChannels(
        PixelImage_Dimension inNumChannels)
    {
        setSizeAndChannels(width, height, inNumChannels);
    }

    inline PixelImage_Dimension PixelImageBase::getWidth() const
    {
        return width;
    }

    inline PixelImage_Dimension PixelImageBase::getHeight() const
    {
        return height;
    }

    inline PixelImage_Dimension PixelImageBase::getChannelCount() const
    {
        return numChannels;
    }

    inline PixelImageBase::PixelImageBase()
    {
        width = 0;
        height = 0;
        numChannels = 0;
    }

    inline PixelImageBase::~PixelImageBase()
    {
    }

    inline size_t PixelImageBase::findIndex(
        PixelImage_Coordinate x,
        PixelImage_Coordinate y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        if(edgeMode == PixelImage_EdgeMode_Clamp) {

            if(x < 0)       x = 0;
            if(y < 0)       y = 0;
            if(x >= width)  x = width - 1;
            if(y >= height) y = height - 1;

            // FIXME: Edge mode applied to channel index? That might not
            // make sense.
            if(channel < 0) channel = 0;
            if(channel >= numChannels) channel = numChannels - 1;

            return
                channel +
                x * numChannels +
                y * numChannels * width;
        }


        // Default to wrapping mode.

        if(x < 0) x = width + (x % width);
        if(y < 0) y = height + (y % height);
        if(x >= PixelImage_Coordinate(width)) x = x % width;
        if(y >= PixelImage_Coordinate(height)) y = y % height;

        // FIXME: Edge mode applied to channel index? That might not make
        // sense.
        if(channel < 0) channel = numChannels + (channel % numChannels);
        if(channel >= PixelImage_Coordinate(numChannels)) channel = channel % numChannels;

        return
            channel +
            x * numChannels +
            y * numChannels * width;
    }

    inline PixelImageBase &PixelImageBase::operator=(const PixelImageBase &other)
    {
        setSizeAndChannels(
            other.getWidth(),
            other.getHeight(),
            other.getChannelCount());

        for(size_t y = 0; y < height; y++) {
            for(size_t x = 0; x < width; x++) {
                for(size_t c = 0; c < numChannels; c++) {
                    setDouble(x, y, c, other.getDouble(x, y, c));
                }
            }
        }

        return *this;
    }

    inline double PixelImageBase::sampleWithHalfPixelOffset(
        float x, float y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        x -= 0.5f / float(getWidth());
        y -= 0.5f / float(getHeight());
        return sample(x, y, channel, edgeMode);
    }

    inline double PixelImageBase::sample(
        float x, float y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        float pixelX = x * float(getWidth());
        float pixelY = y * float(getHeight());

        return getInterpolatedValue(pixelX, pixelY, channel, edgeMode);
    }

    inline double PixelImageBase::getInterpolatedValue(
        float x, float y,
        PixelImage_Coordinate channel,
        PixelImage_EdgeMode edgeMode) const
    {
        PixelImage_Coordinate x0 = PixelImage_Coordinate(floor(x));
        double junk;
        float xs = modf(x, &junk);

        PixelImage_Coordinate y0 = PixelImage_Coordinate(floor(y));
        float ys = modf(y, &junk);

        // Shortcut to speed up operations that generally run on a 1:1
        // input-output pixel correlation.
        if(!xs && !ys)
            return getDouble(x0, y0, channel);

        double ret = 0.0;

        float alphaTotal = 0.0f;

        // // Go through each of the four real pixels we need to sample. NOT
        // // CHANNELS.
        // for(unsigned int i = 0; i < 4; i++) {

        //     // Generate this pixel's coordinate.
        //     bool xd = !!(i & 1);
        //     bool yd = !!(i & 2);
        //     PixelImage_Coordinate rx = (x0 + xd);
        //     PixelImage_Coordinate ry = (y0 + yd);

        //     // s is the scaling value for this as we do our average.
        //     float s =
        //         (xd ? xs : (1.0f - xs)) *
        //         (yd ? ys : (1.0f - ys));

        //     // Accumulate color values.
        //     double p = getDouble(
        //         rx, ry, channel,
        //         edgeMode);

        //     ret += p * s;

        //     alphaTotal += s;
        // }

        double upperVal =
            getDouble(x0,     y0, channel, edgeMode) * (1.0f - xs) +
            getDouble(x0 + 1, y0, channel, edgeMode) * xs;

        double lowerVal =
            getDouble(x0,     y0 + 1, channel, edgeMode) * (1.0f - xs) +
            getDouble(x0 + 1, y0 + 1, channel, edgeMode) * xs;

        return upperVal * (1.0f - ys) + lowerVal * ys;

        assert(alphaTotal == 1.0f);

        return ret;
    }

}


