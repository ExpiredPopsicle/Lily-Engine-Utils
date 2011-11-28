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

#include <string>
#include <iostream>
#include <cmath>
#include <cstring>
#include <cassert>
using namespace std;

#include "matrix.h"

namespace ExPop {

    namespace Math {

        Matrix4x4::Matrix4x4(void) {

            // Set it to an identity matrix
            for(int i = 0; i < 16; i++) {
                if(!(i % 5)) {
                    m[i] = 1;
                } else {
                    m[i] = 0;
                }
            }
        }

        Matrix4x4::Matrix4x4(VEC_SCALAR *src) {
            //memcpy(m, src, sizeof(VEC_SCALAR) * 16);
            for(int x = 0; x < 4; x++) {
                for(int y = 0; y < 4; y++) {
                    m[x + y * 4] = src[y + x * 4];
                }
            }
        }

        VEC_SCALAR *matrixGetRow(VEC_SCALAR *m, int row, int width) {
            return m + row * width;
        }

        void matrixSubtractRow(VEC_SCALAR value, VEC_SCALAR *row1, VEC_SCALAR *row2, int width) {
            int i;
            for(i = 0; i < width; i++) {
                row2[i] -= row1[i] * value;
            }
        }

        void matrixDivideRow(VEC_SCALAR value, VEC_SCALAR *row, int width) {
            int i;
            for(i = 0; i < width; i++) {
                row[i] /= value;
            }
        }

        Matrix4x4 Matrix4x4::inverse(void) const {

            int x, y;
            Matrix4x4 out;
	
            // Start off with a 8x4 matrix, just like if this was being
            // done on paper.
            VEC_SCALAR workMatrix[] = {
                m[0],  m[1],  m[2],  m[3],  1, 0, 0, 0,
                m[4],  m[5],  m[6],  m[7],  0, 1, 0, 0,
                m[8],  m[9],  m[10], m[11], 0, 0, 1, 0,
                m[12], m[13], m[14], m[15], 0, 0, 0, 1,
            };
	
            for(x = 0; x < 4; x++) {

                VEC_SCALAR *curRow = matrixGetRow(workMatrix, x, 8);
		
                // This is a hideous hack to prevent division by zero
                // (which was totally hosing up some matrices) and let us
                // invert some matrices that shouldn't technically be
                // invertable. (Perspective matrix, etc.)
                if(curRow[x] < 0.00000001 && curRow[x] > -0.00000001) curRow[x] = 0.00000001;
			
                // Divide the whole current row by whatever is in its
                // position that's diagonal from the corner. This will
                // result in a 1 in the diagonal slot, and all the other
                // values in the row affected by the same scale.
                matrixDivideRow(curRow[x], curRow, 8);
		
                // Go through all the other rows and subtract this row
                // from it so there's a zero in the COLUMN that
                // corresponds to our current (x) ROW in the ROW we're
                // iterating on (y).
                for(y = 0; y < 4; y++) {
                    if(x != y) {
                        VEC_SCALAR *workRow = matrixGetRow(workMatrix, y, 8);
                        matrixSubtractRow(workRow[x], curRow, workRow, 8);
                    }
                }
            }

            // FIXME: Make this for loop less dumb. Do it next time I
            // actually invert a matrix so I know I haven't hosed
            // something up.

            // Take the right side of our work matrix as the result.
            for(y = 0; y < 4; y++) {
                for(x = 0; x < 8; x++) {
                    //std::cout << workMatrix[x + y * 8] << "\t";
                    if(x >= 4) {
                        out.m[(x - 4) + y * 4] = workMatrix[x + y * 8];
                    }
                }
            }
	
            return out;
        }

        Matrix4x4 Matrix4x4::transpose(void) const {
            Matrix4x4 r;
            for(int x = 0; x < 4; x++) {
                for(int y = 0; y < 4; y++) {
                    r.set(y, x, get(x, y));
                }
            }
            return r;
        }

        Matrix4x4 Matrix4x4::rotationOnly(void) const {
            Matrix4x4 mat = *this;
            mat.set(3, 0, 0);
            mat.set(3, 1, 0);
            mat.set(3, 2, 0);
            mat.set(0, 3, 0);
            mat.set(1, 3, 0);
            mat.set(2, 3, 0);
            mat.set(3, 3, 1);
            return mat;
        }

        Matrix4x4 Matrix4x4::translationOnly(void) const {
            Matrix4x4 mat;
            mat.set(3, 0, get(3, 0));
            mat.set(3, 1, get(3, 1));
            mat.set(3, 2, get(3, 2));
            return mat;
        }

        Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &mat) const {
            // TODO: Implement this
            cout << "WARNING! Matrix addition Unimplemented!" << endl;
            assert(0);
            return Matrix4x4();
        }

        Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &mat) const {
            Matrix4x4 out;
	
            for(int x = 0; x < 4; x++) {
                for(int y = 0; y < 4; y++) {
			
                    VEC_SCALAR r = 0;
			
                    for(int i = 0; i < 4; i++) {
                        r += get(i, y) * mat.get(x, i);
                    }
			
                    out.set(x, y, r);
			
                }
            }
	
            return out;
	
        }

        void debugMatrixPrint(const Matrix4x4 &mat) {
            for(int y = 0; y < 4; y++) {
                for(int x = 0; x < 4; x++) {
                    std::cout << mat.get(x, y) << "\t";
                }
                std::cout << std::endl;
            }
        }


        Vector3D Matrix4x4::operator*(const Vector3D &v) const {
            Vector3D r;
	
            r.x = v.x * m[0] + v.y * m[1] + v.z * m[2]  + m[3];
            r.y = v.x * m[4] + v.y * m[5] + v.z * m[6]  + m[7];
            r.z = v.x * m[8] + v.y * m[9] + v.z * m[10] + m[11];
            VEC_SCALAR div = v.x * m[12] + v.y * m[13] + v.z * m[14] + m[15];
	
            return r / div;
	
        }

        Matrix4x4 translationMatrix(const Vector3D &t) {
            Matrix4x4 mat;
            mat.set(3, 0, t.x);
            mat.set(3, 1, t.y);
            mat.set(3, 2, t.z);
            return mat;
        }

        Matrix4x4 rotationMatrix(const Vector3D &axis, VEC_SCALAR angle) {
            VEC_SCALAR c = cos(angle);
            VEC_SCALAR s = sin(angle);
            VEC_SCALAR t = 1 - cos(angle);
	
            Matrix4x4 mat;
	
            mat.set(0, 0, t*axis.x*axis.x + c);
            mat.set(1, 0, t*axis.x*axis.y + s*axis.z);	
            mat.set(2, 0, t*axis.x*axis.z - s*axis.y);
	
            mat.set(0, 1, t*axis.y*axis.x - s*axis.z);
            mat.set(1, 1, t*axis.y*axis.y + c);	
            mat.set(2, 1, t*axis.y*axis.z + s*axis.x);
	
            mat.set(0, 2, t*axis.z*axis.x + s*axis.y);
            mat.set(1, 2, t*axis.z*axis.y - s*axis.x);	
            mat.set(2, 2, t*axis.z*axis.z + c);
	
            return mat;
        }

        Matrix4x4 uniformScaleMatrix(VEC_SCALAR s) {
            Matrix4x4 mat;
            mat.set(0, 0, s);
            mat.set(1, 1, s);
            mat.set(2, 2, s);
            return mat;
        }

        Matrix4x4 scaleMatrix(const Vector3D &v) {
            Matrix4x4 mat;
            mat.set(0, 0, v.x);
            mat.set(1, 1, v.y);
            mat.set(2, 2, v.z);
            return mat;
        }

        Matrix4x4 perspectiveMatrix(
            VEC_SCALAR fovY,
            VEC_SCALAR aspectRatio,
            VEC_SCALAR zNear,
            VEC_SCALAR zFar) {

            fovY *= 3.14159 / 360.0;

            VEC_SCALAR tmp1 = cos(fovY) / sin(fovY);
            VEC_SCALAR tmp2 = zFar - zNear;

            Matrix4x4 mat;

            mat.set(0, 0, tmp1 / aspectRatio);
            mat.set(1, 1, tmp1);
            mat.set(2, 2, -(zFar + zNear) / tmp2);
            mat.set(3, 2, -1);
            mat.set(2, 3, (-2 * zNear * zFar) / tmp2);
            mat.set(3, 3, 0);

            return mat;

        }

        Matrix4x4 frustumMatrix(
            VEC_SCALAR left,
            VEC_SCALAR right,
            VEC_SCALAR bottom,
            VEC_SCALAR top,
            VEC_SCALAR near,
            VEC_SCALAR far) {

            Matrix4x4 mat;

            mat.set(0, 0, (2.0 * near) / (right - left));
            mat.set(2, 0, (right + left) / (right - left));
            mat.set(1, 1, (2.0 * near) / (top - bottom));
            mat.set(2, 1, (top + bottom) / (top - bottom));
            mat.set(2, 2, -(far + near) / (far - near));
            mat.set(3, 2, -(2.0 * far * near) / (far - near));
            mat.set(2, 3, -1);
            mat.set(3, 3, 0);

            return mat;
        }

    }

}

