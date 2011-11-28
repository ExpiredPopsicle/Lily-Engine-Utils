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

#include <string>

#include "mathdefs.h"
#include "vector3d.h"

namespace ExPop {

    namespace Math {

        /// 4x4 matrix for 3D transforms and junk.
        class Matrix4x4 {
        public:
            Matrix4x4(void);
            Matrix4x4(VEC_SCALAR *src);

            VEC_SCALAR m[16];

            inline VEC_SCALAR get(int x, int y) const {
                return m[x + y * 4];
            }

            inline void set(int x, int y, VEC_SCALAR value) {
                m[x + y * 4] = value;
            }

            Matrix4x4 inverse(void) const;
            Matrix4x4 transpose(void) const;
            Matrix4x4 rotationOnly(void) const;
            Matrix4x4 translationOnly(void) const;
            Matrix4x4 operator+(const Matrix4x4 &mat) const;
            Matrix4x4 operator*(const Matrix4x4 &mat) const;
            Vector3D operator*(const Vector3D &v) const;

        };

        Matrix4x4 translationMatrix(const Vector3D &t);
        Matrix4x4 rotationMatrix(const Vector3D &axis, VEC_SCALAR angle);
        Matrix4x4 uniformScaleMatrix(VEC_SCALAR s);
        Matrix4x4 scaleMatrix(const Vector3D &v);

        Matrix4x4 perspectiveMatrix(
            VEC_SCALAR fovY,
            VEC_SCALAR aspectRatio,
            VEC_SCALAR zNear,
            VEC_SCALAR zFar);

        Matrix4x4 frustumMatrix(
            VEC_SCALAR left,
            VEC_SCALAR right,
            VEC_SCALAR bottom,
            VEC_SCALAR top,
            VEC_SCALAR near,
            VEC_SCALAR far);

        void debugMatrixPrint(const Matrix4x4 &mat);

    }

}

