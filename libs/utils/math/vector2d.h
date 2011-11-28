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

#include <ostream>
#include <cmath>
#include <iostream>
#include <assert.h>
#include <string>

#include "mathdefs.h"

namespace ExPop {

    namespace Math {

        /// Vector in 2D space. (Or: Yet another vector class.)
        /// Mostly inline, so all the implementation is in the header.
        class Vector2D {
        public:

            /** @brief Constructor.
             * @param x The x value of the vector.
             * @param y The y value of the vector.
             * */
            inline Vector2D(VEC_SCALAR x, VEC_SCALAR y) {
                this->x = x;
                this->y = y;
            }

            /** @brief Default constructor.
             * */
            inline Vector2D(void) {
                this->x = 0;
                this->y = 0;
            }

            /// Array constructor.
            inline Vector2D(VEC_SCALAR *f) {
                this->x = f[0];
                this->y = f[1];
            }

            /** @brief Multiplication by scalar.
             * @param s The scalar to multiply by.
             * @return The vector multiplied by the scalar.
             * */
            inline Vector2D operator*(VEC_SCALAR s) const {
                return Vector2D(x * s, y * s);
            }

            /** @brief Division by scalar.
             * @param s The scalar to divide by.
             * @return The vector divided by the scalar.
             * */
            inline Vector2D operator/(VEC_SCALAR s) const {
                return Vector2D(x / s, y / s);
            }

            /** @brief Get the magnitude.
             * @return The magnitude.
             * */
            inline VEC_SCALAR magnitude(void) const {
                return std::sqrt(x * x + y * y);
            }

            /** @brief Get a normalized version of the vector.
             * @return The normalized vector.
             * */
            inline Vector2D normalize(void) const {
                assert(magnitude() != 0); // I'm going to leave this in here for now.
                return (*this)/magnitude();
            }

            /** @brief Subtraction operator.
             * @param v The other vector to subtract.
             * @return The result of the subtraction.
             * */
            inline Vector2D operator-(const Vector2D &v) const {
                return Vector2D(x - v.x, y - v.y);
            }

            /** @brief Addition operator.
             * @param v The other vector to add.
             * @return The result of the addition.
             * */
            inline Vector2D operator+(const Vector2D &v) const {
                return Vector2D(x + v.x, y + v.y);
            }

            /** @brief Dot product operation.
             * @param v The other vector to dot with.
             * @return The dot product of this vector and the other.
             * */
            inline VEC_SCALAR dot(const Vector2D &v) const {
                return x * v.x + y * v.y;
            }

            /** @brief Addition and assignment operator.
             * @param v The other vector to add.
             * @return This vector after the operation.
             * */
            inline Vector2D &operator+=(const Vector2D &v) {
                x += v.x;
                y += v.y;
                return *this;
            }

            /** @brief Subtraction and assignment operator.
             * @param v The other vector to subtract.
             * @return This vector after the operation.
             * */
            inline Vector2D &operator-=(const Vector2D &v) {
                x -= v.x;
                y -= v.y;
                return *this;
            }

            /** @brief Multiplication by scalar and assignment operation.
             * @param s The scalar to multiply by.
             * @return This vector after the operation.
             * */
            inline Vector2D &operator*=(VEC_SCALAR s) {
                x *= s;
                y *= s;
                return *this;
            }

            /** @brief Comparison operator.
             * @param v The other vector to compare to.
             * @return true if they're the same, false otherwise.
             * */
            inline bool operator==(const Vector2D &v) const {
                return (x == v.x && y == v.x);
            }

            /** @brief Checks to see if it's zero length.
             * @return true if it's zero magnitude. false otherwise.
             * */
            inline bool isZero(void) const {
                return x == 0 && y == 0;
            }

            /** @brief The x value of the vector. */
            VEC_SCALAR x;

            /** @brief The y value of the vector. */
            VEC_SCALAR y;
        };

        /// Stream output operator.
        inline std::ostream &operator<<(std::ostream &os, const Vector2D &v) {
            os << v.x << " " << v.y;
            return os;
        }

        /// Stream input operator.
        inline std::istream &operator>>(std::istream &is, Vector2D &v) {
            is >> v.x >> v.y;
            return is;
        }

        /** @brief Get a vector that's perpendicular to the given one.
         * @param p The input vector.
         * @return The perpendicular vector.
         * */
        inline Vector2D getPerpendicularAxis(const Vector2D &p) {
            return Vector2D(p.y, -p.x);
        }

        /** @brief Projects a point onto an axis.
         * Just does a dot product with some automatic normalization first.
         * @return The resulting point.
         * */
        inline VEC_SCALAR projectPointOnAxis(const Vector2D &p1, const Vector2D &axis) {
            Vector2D realAxis = axis.normalize();
            return p1.x * realAxis.x + p1.y * realAxis.y;
        }

    }

}
