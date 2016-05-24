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

#include <cmath>
#include <iostream>
using namespace std;

#include "angle.h"

namespace ExPop {

    Angle::Angle(void)
    {
        angleDegrees = 0.0f;
    }

    Angle::Angle(float degrees)
    {
        setDegrees(degrees);
    }

    float Angle::getDegrees(void) const
    {
        return angleDegrees;
    }

    float Angle::getRadians(void) const
    {
        return angleDegrees * 3.14159f/180.0f;
    }

    void Angle::setDegrees(float degrees)
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

    void Angle::setRadians(float radians)
    {
        setDegrees(radians * 180.0f/3.14159f);
    }

    Angle interpAngle(const Angle &a, const Angle &b, float alpha)
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
