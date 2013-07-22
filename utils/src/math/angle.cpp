#include <cmath>
#include <iostream>
using namespace std;

#include "angle.h"

namespace ExPop {

    Angle::Angle(void) {
        angleDegrees = 0.0f;
    }

    Angle::Angle(float degrees) {
        setDegrees(degrees);
    }

    float Angle::getDegrees(void) const {
        return angleDegrees;
    }

    float Angle::getRadians(void) const {
        return angleDegrees * 3.14159f/180.0f;
    }

    void Angle::setDegrees(float degrees) {
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

    void Angle::setRadians(float radians) {
        setDegrees(radians * 180.0f/3.14159f);
    }

    Angle interpAngle(const Angle &a, const Angle &b, float alpha) {
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
