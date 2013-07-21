#pragma once

namespace ExPop {

    /// Angle class. Keeps everything nicely in the 0-360 degree
    /// range. Also handles interpolation so you can always
    /// interpolate across the smallest possible arc.
    class Angle {
    public:

        Angle(void);
        Angle(float degrees);

        float getDegrees(void) const;
        float getRadians(void) const;

        void setDegrees(float degrees);
        void setRadians(float radians);

    private:

        // We store it as degrees internally because it makes the
        // wrapping magic a lot easier to deal with.
        float angleDegrees;
    };

    Angle interpAngle(const Angle &a, const Angle &b, float alpha);

    inline std::ostream &operator<<(std::ostream &out, const Angle &angle) {
        out << angle.getDegrees();
        return out;
    }

    inline std::istream &operator>>(std::istream &in, Angle &angle) {
        float degrees;
        in >> degrees;
        angle.setDegrees(degrees);
        return in;
    }

}
