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

// Generic single-channel pixel data type. Can map different data
// types to float data, like the 0 to 255 range of a uint8_t value
// that needs to be mapped to 0.0 to 1.0.

// Lots of magic typecasting and operator overloading happens here to
// make the mapping as transparent as possible. Raw data can still be
// accessed (and modified!) with the public "value" field.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Scaling modes to bring arbitrary types into a normalized range (0
    /// = black, 1 = white). Each one of these needs a corresponding entry
    /// in PixelValue::getValueAtOne() to function.
    enum ScalingType
    {
        /// Map stuff linearly. Useful for float types.
        ScalingType_OneIsOne,

        /// 1.0 scaled matches the max value representable in the integer
        /// type (and -1.0 scaled matches the lowest on signed integers).
        /// 0 is still 0. This is for getting the full range out of your
        /// integer types mapped to a 0 to 1 or -1 to 1 range (signed
        /// types) when scaled.
        ScalingType_OneIsMaxInt,

        /// This is what you'd use if you want the 0-255 range mapped to
        /// 0.0-1.0, but with the ability to go over like with an HDR
        /// buffer.
        ScalingType_OneIs255

    };

    /// Get the default scaling mode for a given type. In general, ints
    /// are given a mode to let them use the full range of the bits
    /// available to represent 0 to 1 or -1 to 1 (depending on if they're
    /// signed or not) (ScalingType_OneIsMaxInt). Floats and doubles are
    /// given 1:1 scaling (ScalingType_OneIsOne).
    template<typename ValueType>
    inline constexpr ScalingType pixelValueGetDefaultScalingType();

    /// Pixel value type. Represents a color value with a various ranges,
    /// for example an 8-bit value that represents the range of 0 to 1 for
    /// a single color channel.
    template<
        typename ValueType,
        ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    class PixelValue
    {
    public:

        /// Basic constructor.
        PixelValue();

        /// Copy constructor.
        template<class OtherValueType, ScalingType otherScalingType>
        PixelValue(const PixelValue<OtherValueType, otherScalingType> &other);

        /// Generic initialization from arbitrary types. Scaled.
        template<typename ScaledType>
        PixelValue(const ScaledType &inScaledValue);

        /// The value itself. This should be the only data in this entire
        /// class, so we can use arrays of ValueType interchangeably with
        /// this. (PixelValue must be able to be passed into GL texture
        /// API, etc.)
        ValueType value;

        /// Get a color value, scaled by the scaling mode.
        template<typename ScaledType>
        ScaledType getScaledValue() const;

        /// Set a color value, clamped to the min/max range representable
        /// by the internal data type and the scaling parameter.
        template<typename ScaledType>
        void setScaledValue(ScaledType inVal);

        /// Copy another value, scaling as necessary. Note: Unintended
        /// precision loss is possible when copying between values with
        /// different ValueTypes or ScalingModes.
        template<class OtherValueType, ScalingType otherScalingType>
        PixelValue<ValueType, scalingType> &operator=(const PixelValue<OtherValueType, otherScalingType> &other);

        /// Typecast operator, for easy conversion to other PixelValue
        /// types.
        template<class OtherValueType, ScalingType otherScalingType>
        operator PixelValue<OtherValueType, otherScalingType>();

        /// Typecast operator, for easy conversion to float types or
        /// whatever.
        template<typename ScaledType>
        operator ScaledType();

        /// Implicit incoming conversion operator.
        template<typename ScaledType>
        PixelValue<ValueType, scalingType> &operator=(const ScaledType &other);

        // ----------------------------------------------------------------------
        // Static type information convenience stuff

        /// Get the highest value we can represent after scaling.
        template<typename ScaledType>
        static ScaledType getMaxScaled();

        /// Get the lowest value we can represent after scaling.
        template<typename ScaledType>
        static ScaledType getLowestScaled();

        /// Get the value internally that represents 1 when scaled.
        static inline constexpr ValueType getValueAtOne();

        // ----------------------------------------------------------------------
        // Math operators

        // TODO: A bunch of stuff that casts/scales to double, then does
        // the math, then returns a value of this type with the result.
    };

    /// Make a pixel with some raw data. We don't have a PixelValue
    /// constructor that does this because we have a constructor that
    /// takes scaled data of arbitrary types already. So use this in place
    /// of that constructor.
    template<class ValueType, ScalingType scalingType = pixelValueGetDefaultScalingType<ValueType>()>
    PixelValue<ValueType, scalingType> rawPixelValue(const ValueType &v);

}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    // operator << for ostreams
    template<typename ValueType, ScalingType scalingType>
    inline std::ostream &operator<<(std::ostream &ostr, PixelValue<ValueType, scalingType> &pixelValue)
    {
        ostr << pixelValue.template getScaledValue<double>();
        return ostr;
    }

    // operator >> for istreams
    template<typename ValueType, ScalingType scalingType>
    inline std::istream &operator>>(std::istream &istr, PixelValue<ValueType, scalingType> &pixelValue)
    {
        double tmp = 0.0;
        istr >> tmp;
        pixelValue.template setScaledValue<double>(tmp);
        return istr;
    }

    // operator= from generic type
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline PixelValue<ValueType, scalingType> &PixelValue<ValueType, scalingType>::operator=(const ScaledType &other)
    {
        setScaledValue<ScaledType>(other);
        return *this;
    }

    // typecast to generic type
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline PixelValue<ValueType, scalingType>::operator ScaledType()
    {
        return getScaledValue<ScaledType>();
    }

    // operator PixelValue (different format)
    template<typename ValueType, ScalingType scalingType>
    template<class OtherValueType, ScalingType otherScalingType>
    inline PixelValue<ValueType, scalingType>::operator PixelValue<OtherValueType, otherScalingType>()
    {
        PixelValue<OtherValueType, otherScalingType> ret;
        copyValue(*this, ret);
        return ret;
    }

    // pixelValueGetDefaultScalingType
    template<typename ValueType>
    inline constexpr ScalingType pixelValueGetDefaultScalingType()
    {
        // For int types, default to using the whole range. For floats
        // (and others), just pass through.
        return std::is_integral<ValueType>::value ? ScalingType_OneIsMaxInt : ScalingType_OneIsOne;
    }

    // setScaledValue
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline void PixelValue<ValueType, scalingType>::setScaledValue(ScaledType inVal)
    {
        if(inVal >= getMaxScaled<ScaledType>()) {
            value = std::numeric_limits<ValueType>::max();
        } else if(inVal <= getLowestScaled<ScaledType>()) {
            value = std::numeric_limits<ValueType>::lowest();
        } else {
            if(std::is_integral<ValueType>::value && !std::is_integral<ScaledType>::value) {
                value = ValueType(std::round(inVal * getValueAtOne()));
            } else {
                value = ValueType(inVal * getValueAtOne());
            }
        }
    }

    // getScaledValue
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline ScaledType PixelValue<ValueType, scalingType>::getScaledValue() const
    {
        ScaledType a = ScaledType(value) / ScaledType(getValueAtOne());
        return a;
    }

    // getLowestScaled
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline ScaledType PixelValue<ValueType, scalingType>::getLowestScaled()
    {
        return ScaledType(std::numeric_limits<ValueType>::lowest()) / ScaledType(getValueAtOne());
    }

    // getMaxScaled
    template<typename ValueType, ScalingType scalingType>
    template<typename ScaledType>
    inline ScaledType PixelValue<ValueType, scalingType>::getMaxScaled()
    {
        return ScaledType(std::numeric_limits<ValueType>::max()) / ScaledType(getValueAtOne());
    }

    // getValueAtOne
    template<typename ValueType, ScalingType scalingType>
    inline constexpr ValueType PixelValue<ValueType, scalingType>::getValueAtOne()
    {
        // This only works in C++14.

        // switch(scalingType) {
        //     case ScalingType_OneIsOne:
        //         return ValueType(1);
        //     case ScalingType_OneIsMaxInt:
        //         return std::numeric_limits<ValueType>::max();
        //     case ScalingType_OneIs255:
        //         return 255;
        // }
        // return ValueType(1);

        // This is fine in C++11.
        return
            scalingType == ScalingType_OneIsOne    ? ValueType(1) :
            scalingType == ScalingType_OneIsMaxInt ? std::numeric_limits<ValueType>::max() :
            scalingType == ScalingType_OneIs255    ? 255 :
            ValueType(1);
    }

    // Non-template-specialized value copy that just uses double as an
    // intermediary.
    template<
        typename SrcValueType, ScalingType SrcScalingType,
        typename DstValueType, ScalingType DstScalingType>
    inline void copyValue(
        const PixelValue<SrcValueType, SrcScalingType> &src,
        PixelValue<DstValueType, DstScalingType> &dst)
    {
        // This is the first time I've had to use this syntax. Weird.

        // FIXME: This is pretty gross. Precision loss and speed loss will
        // happen in some cases. We should explicitly handle more cases
        // where we know we can copy the values across directly.
        dst.template setScaledValue<double>(
            src.template getScaledValue<double>());
    }

    // Template-specialized version for identical types that copies a
    // value directly.
    template<
        typename SrcValueType, ScalingType SrcScalingType>
    inline void copyValue(
        const PixelValue<SrcValueType, SrcScalingType> &src,
        PixelValue<SrcValueType, SrcScalingType> &dst)
    {
        dst.value = src.value;
    }

    // operator=
    template<class ValueType, ScalingType scalingType>
    template<class OtherValueType, ScalingType otherScalingType>
    inline PixelValue<ValueType, scalingType>& PixelValue<ValueType, scalingType>::operator=(const PixelValue<OtherValueType, otherScalingType> &other)
    {
        copyValue(other, *this);
        return *this;
    }

    // Base constructor
    template<class ValueType, ScalingType scalingType>
    PixelValue<ValueType, scalingType>::PixelValue()
    {
        value = 0;
    }

    // Copy constructor
    template<class ValueType, ScalingType scalingType>
    template<class OtherValueType, ScalingType otherScalingType>
    PixelValue<ValueType, scalingType>::PixelValue(
        const PixelValue<OtherValueType, otherScalingType> &other)
    {
        copyValue(other, *this);
    }

    // Scaled data constructor
    template<class ValueType, ScalingType scalingType>
    template<typename ScaledType>
    PixelValue<ValueType, scalingType>::PixelValue(const ScaledType &inScaledValue)
    {
        setScaledValue<ScaledType>(inScaledValue);
    }

    // rawPixelValue
    template<class ValueType, ScalingType scalingType>
    PixelValue<ValueType, scalingType> rawPixelValue(const ValueType &v)
    {
        PixelValue<ValueType, scalingType> ret;
        ret.value = v;
        return ret;
    }
}
