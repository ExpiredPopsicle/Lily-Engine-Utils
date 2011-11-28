// ---------------------------------------------------------------------------
//
//   Lily Engine alpha
//
//   Copyright (c) 2010 Clifford Jolly
//     http://expiredpopsicle.com
//     expiredpopsicle@gmail.com
//
// ---------------------------------------------------------------------------
//
//   Copyright (c) 2011 Clifford Jolly
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

#pragma once

#include <cmath>
#include <iostream>

#include "mathdefs.h"

namespace ExPop {

    namespace Math {

        /// 3D vector.
        class Vector3D {

        public:

            VEC_SCALAR x, y, z;

            Vector3D(void) {
                x = y = z = 0;
            }

            inline Vector3D(VEC_SCALAR x, VEC_SCALAR y, VEC_SCALAR z) {
                this->x = x;
                this->y = y;
                this->z = z;
            }

            inline Vector3D(VEC_SCALAR *f) {
                this->x = f[0];
                this->y = f[1];
                this->z = f[2];
            }

            inline Vector3D operator+(const Vector3D &v) const {
                Vector3D r;

                r.x = v.x + x;
                r.y = v.y + y;
                r.z = v.z + z;

                return r;
            }

            inline Vector3D operator-(const Vector3D &v) const {
                Vector3D r;

                r.x = v.x - x;
                r.y = v.y - y;
                r.z = v.z - z;

                return r;
            }

            inline Vector3D operator*(VEC_SCALAR s) const {
                Vector3D r;

                r.x = x * s;
                r.y = y * s;
                r.z = z * s;

                return r;
            }

            inline Vector3D operator/(VEC_SCALAR s) const {
                Vector3D r;

                r.x = x / s;
                r.y = y / s;
                r.z = z / s;

                return r;
            }

            inline Vector3D cross(const Vector3D &v) const {
                Vector3D r;

                r.x =  y   * v.z - z   *  v.y;
                r.y = -v.z * x   - v.x * -z;
                r.z =  x   * v.y - y   *  v.x;
                return r;
            }

            /// Dot product.
            inline VEC_SCALAR dot(const Vector3D &v) const {
                return v.x * x + v.y * y + v.z * z;
            }

            inline VEC_SCALAR magnitude(void) const {
                return std::sqrt(x * x + y * y + z * z);
            }

            inline bool operator==(const Vector3D &v) const {
                return v.x == x && v.y == y && v.z == z;
            }

            // TODO: Need a ostream/istream operators and tinyXML load/save
            //   like Vector2D
        };

        /// Stream output operator.
        inline std::ostream &operator<<(std::ostream &os, const Vector3D &v) {
            os << v.x << " " << v.y << " " << v.z;
            return os;
        }

        /// Stream input operator.
        inline std::istream &operator>>(std::istream &is, Vector3D &v) {
            is >> v.x >> v.y >> v.z;
            return is;
        }

    }

}
