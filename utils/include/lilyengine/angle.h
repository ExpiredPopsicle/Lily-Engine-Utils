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

// I got sick of dealing with interpolation between two angles across
// their shortest arc, and kept writing the same ugly chunk of code
// over and over again. So here's an implementation of an angle type
// that reduces angles to the 0-360 degree range, and allows
// interpolation across the shortest arc.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

#include <sstream>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Angle class. Keeps everything nicely in the 0-360 degree
    /// range. Also handles interpolation so you can always
    /// interpolate across the smallest possible arc.
    class Angle
    {
    public:

        /// Basic constructor initializes angle to zero degrees.
        Angle(void);

        /// Take an angle in degrees as a constructor.
        Angle(float degrees);

        // Note: Default copy constructors are fine here.

        /// Get angle as degrees.
        float getDegrees(void) const;

        /// Get angle as radians (converts).
        float getRadians(void) const;

        /// Set angle as degrees.
        void setDegrees(float degrees);

        /// Set angle as radians (converts).
        void setRadians(float radians);

        // TODO: Implement operator overloading so we can more easily
        // do simple math with angles.

    private:

        // We store it as degrees internally because it makes the
        // wrapping magic a lot easier to deal with.
        float angleDegrees;
    };

    /// Interpolate angles.
    inline Angle interpAngle(const Angle &a, const Angle &b, float alpha);

    /// Output an angle to an ostream (as degrees).
    inline std::ostream &operator<<(std::ostream &out, const Angle &angle);

    /// Read an angle from an ostream (as degrees).
    inline std::istream &operator>>(std::istream &in, Angle &angle);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    inline std::ostream &operator<<(std::ostream &out, const Angle &angle)
    {
        out << angle.getDegrees();
        return out;
    }

    inline std::istream &operator>>(std::istream &in, Angle &angle)
    {
        float degrees;
        in >> degrees;
        angle.setDegrees(degrees);
        return in;
    }

    inline Angle::Angle(void)
    {
        angleDegrees = 0.0f;
    }

    inline Angle::Angle(float degrees)
    {
        setDegrees(degrees);
    }

    inline float Angle::getDegrees(void) const
    {
        return angleDegrees;
    }

    inline float Angle::getRadians(void) const
    {
        return angleDegrees * 3.14159f/180.0f;
    }

    inline void Angle::setDegrees(float degrees)
    {
        angleDegrees = degrees;

        if(angleDegrees >= 360.0f) {
            angleDegrees -= floor(angleDegrees / 360.0f) * 360.0f;
        }

        if(angleDegrees < 0.0f) {
            angleDegrees = angleDegrees + fabs(floor(angleDegrees/360.0f) * 360.0f);
        }

        if(angleDegrees == -0.0f) {
            angleDegrees = 0.0f;
        }
    }

    inline void Angle::setRadians(float radians)
    {
        setDegrees(radians * 180.0f/3.14159f);
    }

    inline Angle interpAngle(const Angle &a, const Angle &b, float alpha)
    {
        float degrees0 = a.getDegrees();
        float degrees1 = b.getDegrees();

        // Find an offset to degrees1 to bring it closer to degrees0
        // by wrapping around.
        if(fabs((degrees1 + 360) - degrees0) <
           fabs(degrees1 - degrees0)) {
            degrees1 += 360;
        }
        if(fabs((degrees1 - 360) - degrees0) <=
           fabs(degrees1 - degrees0)) {
            degrees1 -= 360;
        }

        // Now we can linearly interpolate, and rely on the angle set
        // in the constructor to properly wrap around.
        return Angle(degrees0 * (1.0f - alpha) + degrees1 * alpha);
    }
}
