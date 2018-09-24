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

// I wrote a matrix class. I know there are tons of them out there,
// but this one is mine. Should do most of the usual stuff. Helper
// functions exist for constructing frustum matrices, rotation
// matrices, scaling matrices, and translation matrices.

// ----------------------------------------------------------------------
// Needed headers
// ----------------------------------------------------------------------

#pragma once

// We do some bounds checking by default with matrices. This can be
// disabled for an extra little speed boost, but you should always
// test with it enabled.
#ifndef EXPOP_MATRIX_CHECKS
#define EXPOP_MATRIX_CHECKS 1
#endif
#if EXPOP_MATRIX_CHECKS
#include <cassert>
#define EXPOP_MATRIX_ASSERT(x) assert(x)
#else
#define EXPOP_MATRIX_ASSERT(x)
#endif

#define EXPOP_MATRIX_ASSERT_STATIC(x) static_assert(x, "Bad matrix operation")

#include <cmath>
#include <iomanip>
#include <iostream>

// ----------------------------------------------------------------------
// Declarations and documentation
// ----------------------------------------------------------------------

namespace ExPop
{
    /// Matrix/Vector class. All template-tastic so we can use it for
    /// whatever with a single set of code.
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    class Matrix
    {
    public:

        /// This is for convenience so we don't have to keep typing
        /// out the stupid template over and over again for all the
        /// internal junk.
        typedef Matrix<MatScalar, ROWS, COLS> MyType;

        // ----------------------------------------------------------------------
        // Data storage.
        // ----------------------------------------------------------------------

        // This must be the ONLY DATA MEMBER.
        union
        {
            /// Actual data goes here.
            MatScalar data[ROWS * COLS];
        };

        // ----------------------------------------------------------------------
        // Various get/set stuff to mess with the data buffer.
        // ----------------------------------------------------------------------

        inline MatScalar &x(void);
        inline MatScalar &y(void);
        inline MatScalar &z(void);
        inline MatScalar &w(void);

        /// data is a public member, so you don't really have to use
        /// this, but you should because we can turn on error checking
        /// for it.
        inline MatScalar &operator[](unsigned int index);
        inline const MatScalar &operator[](unsigned int index) const;

        /// Get a specific cell. Checks ranges if EXPOP_MATRIX_CHECKS.
        inline MatScalar &get(unsigned int row, unsigned int col);

        /// Get in a more const-y way.
        const inline MatScalar &getConst(unsigned int row, unsigned int col) const;

        /// The () operator just grabs elements by row and column.
        inline MatScalar &operator()(unsigned int row, unsigned int col);

        // ----------------------------------------------------------------------
        // Constructors.
        // ----------------------------------------------------------------------

        /// Default constructor. Makes a unit matrix. Or tries to,
        /// anyway. Might end up weird with non-symmetrical matrices.
        inline Matrix(void);

        /// Matrix from an array.
        inline Matrix(const MatScalar inData[ROWS * COLS]);

        /// Matrix from a Matrix.
        inline Matrix(const MyType &mat);

        /// Constructors for vector types so we don't have to bother
        /// with funky syntax.
        inline Matrix(MatScalar x, MatScalar y);
        inline Matrix(MatScalar x, MatScalar y, MatScalar z);
        inline Matrix(MatScalar x, MatScalar y, MatScalar z, MatScalar w);

        /// Initialize a matrix from another matrix of a different
        /// size. This is so we can do things like get a 3x3 matrix
        /// from a 4x4 matrix or the other way around when doing junk
        /// with homogeneous coordinates. Fills in unit matrix values
        /// anywhere that the values in the other matrix don't exist
        /// and cuts off anything there's not room for in this matrix.
        inline Matrix(
            const MatScalar *otherMatrixData,
            unsigned int otherMatrixRows,
            unsigned int otherMatrixCols);

        /// Initialize a Matrix to one value for every cell.
        inline Matrix(MatScalar v);

        // ----------------------------------------------------------------------
        // Transforms for this one Matrix.
        // ----------------------------------------------------------------------

        /// Matrix transpose.
        inline Matrix<MatScalar, COLS, ROWS> transpose() const;

        /// Subtract row from another row. Modifies the Matrix. (row1
        /// -= row2 * d).
        inline void subtractRow(unsigned int rowNum1, unsigned int rowNum2, MatScalar d);

        /// Divide all the values in a row by a value. Modifies the
        /// Matrix.
        inline void divideRow(unsigned int rowNum, MatScalar d);

        /// Swap two rows. Modifies the Matrix.
        inline void swapRows(unsigned int rowNum1, unsigned int rowNum2);

        /// Invert a Matrix. Matrix must have the same number of rows
        /// and columns. If the Matrix is not invertible, it resorts
        /// to ugly hacks to prevent dividing by zero.
        inline MyType inverse() const;

        // ----------------------------------------------------------------------
        // Now some math operations...
        // ----------------------------------------------------------------------

        /// Add operator.
        inline MyType operator+(const MyType &other) const;

        /// Addition assignment operator.
        inline const MyType &operator+=(const MyType &other);

        /// Subtract operator.
        inline MyType operator-(const MyType &other) const;

        /// Subtract assignment operator.
        inline const MyType &operator-=(const MyType &other);

        /// Basic multiplication. Only works on matrices with equal
        /// rows and columns. More fancy template version later. But I
        /// imagine I'll be using this all the time for 4x4 and 3x3
        /// matrices.
        inline MyType operator*(const MyType &otherMat) const;

        /// Multiply by a differently sized Matrix or multiply
        /// matrices that have width and height that are different.
        /// Output will have number of rows equal to number of rows of
        /// the first, and columns of the second.
        template<unsigned int otherRows, unsigned int otherCols>
        inline Matrix<MatScalar, ROWS, otherCols> multiply(
            const Matrix<MatScalar, otherRows, otherCols> &otherMat) const;

        /// Uniform scaling.
        inline MyType operator*(MatScalar s) const;

        /// Uniform scaling assignment.
        inline const MyType &operator*=(MatScalar s);

        /// Divide by scalar.
        inline MyType operator/(MatScalar s) const;

        /// Vector magnitude.
        inline MatScalar magnitude(void) const;

        /// Vector magnitude without the square root.
        inline MatScalar magnitudeSquared(void) const;

        /// Vector normalize.
        inline MyType normalize(void) const;

        /// Dot product.
        inline MatScalar dot(const MyType &otherMat) const;

        /// Simple test to see if everything is zero.
        inline bool isZero(void) const;

        /// Equality test.
        inline bool operator==(const MyType &otherMat) const;

        /// Non-equality test.
        inline bool operator!=(const MyType &otherMat) const;

        /// Cross product.
        inline MyType crossProduct(const MyType &v) const;

        /// Get number of rows.
        static inline size_t getNumRows();

        /// Get number of columns.
        static inline size_t getNumCols();

        // ----------------------------------------------------------------------
        // Debug junk.
        // ----------------------------------------------------------------------

        inline void debugDump(void) const;

    private:

    };

    // Various commonly used types.
    typedef Matrix<float, 4, 4> FMatrix4x4;
    typedef Matrix<float, 3, 3> FMatrix3x3;
    typedef Matrix<float, 2, 1> FVec2;
    typedef Matrix<float, 3, 1> FVec3;
    typedef Matrix<float, 4, 1> FVec4;
    typedef Matrix<float, 4, 1> FColor4;

    // Utility/helper functions.

    /// Make a 4x4 float perspective matrix. (Recommended to use
    /// makeFrustumMatrix instead!)
    inline FMatrix4x4 makePerspectiveMatrix(
        float fovY,
        float aspectRatio,
        float zNear,
        float zFar);

    /// Make a 4x4 float frustum matrix.
    inline FMatrix4x4 makeFrustumMatrix(
        float left,
        float right,
        float bottom,
        float top,
        float nearDist,
        float farDist);

    /// Make a 4x4 float translation matrix.
    inline FMatrix4x4 makeTranslationMatrix(const FVec3 &t);

    /// Make a 4x4 float rotation matrix.
    inline FMatrix4x4 makeRotationMatrix(const FVec3 &axis, float angle);

    /// Make a 3x3 float rotation matrix around the Z axis.
    inline FMatrix3x3 make2DRotationMatrix(float angle);

    /// Make a 4x4 float scaling matrix.
    inline FMatrix4x4 makeScaleMatrix(const FVec3 &scale);

    /// std::ostream output.
    template <typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline std::ostream &operator<<(std::ostream &out, const Matrix<MatScalar, ROWS, COLS> &mat);

    /// std::istream input.
    template <typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline std::istream &operator>>(std::istream &in, Matrix<MatScalar, ROWS, COLS> &mat);
}

// ----------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------

namespace ExPop
{
    // x()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::x(void)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS > 0);
        return data[0];
    }

    // y()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::y(void)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS > 1);
        return data[1];
    }

    // z()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::w(void)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS > 2);
        return data[2];
    }

    // w()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::z(void)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS > 3);
        return data[3];
    }

    // operator[]
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::operator[](
        unsigned int index)
    {
        EXPOP_MATRIX_ASSERT(index < ROWS * COLS);
        return data[index];
    }

    // operator[], const version
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline const MatScalar &Matrix<MatScalar, ROWS, COLS>::operator[](
        unsigned int index) const
    {
        EXPOP_MATRIX_ASSERT(index < ROWS * COLS);
        return data[index];
    }

    // get
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::get(
        unsigned int row,
        unsigned int col)
    {
        EXPOP_MATRIX_ASSERT(row < ROWS && col < COLS);
        return data[row + (col * ROWS)];
    }

    // getConst
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline const MatScalar &Matrix<MatScalar, ROWS, COLS>::getConst(
        unsigned int row,
        unsigned int col) const
    {
        EXPOP_MATRIX_ASSERT(row < ROWS && col < COLS);
        return data[row + (col * ROWS)];
    }

    // operator()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar &Matrix<MatScalar, ROWS, COLS>::operator()(
        unsigned int row,
        unsigned int col)
    {
        return get(row, col);
    }

    // Matrix()
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(void)
    {
        unsigned int index = 0;

        for(unsigned int col = 0; col < COLS; col++) {

            for(unsigned int row = 0; row < ROWS; row++) {

                // Just set 1 to the diagonals, 0 everywhere else.
                data[index] = (row == col);

                index++;

            }
        }
    }

    // Matrix(array)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(const MatScalar inData[ROWS * COLS])
    {
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                data[index] = inData[index];
                index++;
            }
        }
    }

    // Matrix(otherMatrix)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(const MyType &mat)
    {
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                data[index] = mat.data[index];
                index++;
            }
        }
    }

    // Matrix(data, rows, cols)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(
        const MatScalar *otherMatrixData,
        unsigned int otherMatrixRows,
        unsigned int otherMatrixCols)
    {
        unsigned int index = 0;

        for(unsigned int col = 0; col < COLS; col++) {

            for(unsigned int row = 0; row < ROWS; row++) {

                if(row < otherMatrixRows && col < otherMatrixCols) {
                    data[index] = otherMatrixData[row + (col * otherMatrixRows)];
                } else {
                    data[index] = (row == col);
                }

                index++;

            }
        }

    }

    // Matrix(scalar)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(MatScalar v)
    {
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                data[index] = v;
                index++;
            }
        }
    }

    // Matrix(x, y)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(MatScalar x, MatScalar y)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS == 2);
        data[0] = x;
        data[1] = y;
    }

    // Matrix(x, y, z)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(MatScalar x, MatScalar y, MatScalar z)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS == 3);
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    // Matrix(x, y, z, w)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS>::Matrix(MatScalar x, MatScalar y, MatScalar z, MatScalar w)
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS == 4);
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    // transpose
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, COLS, ROWS> Matrix<MatScalar, ROWS, COLS>::transpose() const
    {
        MyType out;
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                out.data[index] = getConst(col, row);
                index++;
            }
        }
        return out;
    }

    // subtractRow
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline void Matrix<MatScalar, ROWS, COLS>::subtractRow(
        unsigned int rowNum1,
        unsigned int rowNum2,
        MatScalar d)
    {
        for(unsigned int i = 0; i < COLS; i++) {
            get(rowNum1, i) -= get(rowNum2, i) * d;
        }
    }

    // divideRow
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline void Matrix<MatScalar, ROWS, COLS>::divideRow(
        unsigned int rowNum,
        MatScalar d)
    {
        for(unsigned int i = 0; i < COLS; i++) {
            get(rowNum, i) /= d;
        }
    }

    // swapRows
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline void Matrix<MatScalar, ROWS, COLS>::swapRows(
        unsigned int rowNum1,
        unsigned int rowNum2)
    {
        for(unsigned int i = 0; i < COLS; i++) {
            MatScalar tmp = get(rowNum1, i);
            get(rowNum1, i) = get(rowNum2, i);
            get(rowNum2, i) = tmp;
        }
    }

    // inverse
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::inverse() const
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS == COLS);

        MyType out;

        // Make a new matrix that's twice the number of columns so
        // we can do the work inside it.
        Matrix<MatScalar, ROWS, COLS * 2> workMatrix(data, ROWS, COLS);

        // Initialize the right side of this thing to an identity
        // matrix.
        for(unsigned int row = 0; row < ROWS; row++) {
            for(unsigned int col = 0; col < COLS; col++) {
                workMatrix(row, col + COLS) = (row == col);
            }
        }

        for(unsigned int workRow = 0; workRow < ROWS; workRow++) {

            // Find a row without a zero for this slot and swap
            // this row with it.
            unsigned int swapRow = workRow;
            while(swapRow < ROWS && workMatrix(swapRow, workRow) == 0) {
                swapRow++;
            }

            if(swapRow == ROWS) {

                // Didn't find the row we need. Just use the row
                // we started with and jam in some values to work
                // around this. (UGLY HACK ALERT!)
                swapRow = workRow;
                workMatrix(workRow, workRow) = 0.00000001;

            } else if(swapRow != workRow) {

                // We did find a suitable row, but it's not the
                // one we're working on.
                workMatrix.swapRows(swapRow, workRow);

            }

            float val = workMatrix(workRow, workRow);

            // Divide this whole row by the value in workRow,
            // workRow so we have a 1 in the diagonal slot.
            workMatrix.divideRow(workRow, val);

            // Now subtract this row from all other rows, scaled
            // to an appropriate value to eliminate the value from
            // this column.
            for(unsigned int subRow = 0; subRow < ROWS; subRow++) {
                if(subRow != workRow) {
                    workMatrix.subtractRow(subRow, workRow, workMatrix(subRow, workRow));
                }
            }

        }

        // Now take all the values from the right side of
        // workMatrix and stick them into their own Matrix.
        for(unsigned int row = 0; row < ROWS; row++) {
            for(unsigned int col = 0; col < COLS; col++) {
                out(row, col) = workMatrix(row, col + COLS);
            }
        }

        return out;
    }

    // debugDump
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline void Matrix<MatScalar, ROWS, COLS>::debugDump(void) const
    {
        std::cout << "Matrix dump (" << ROWS << "x" << COLS << "):" << std::endl;
        int index = 0;
        for(unsigned int row = 0; row < ROWS; row++) {
            for(unsigned int col = 0; col < COLS; col++) {
                std::cout << std::setw(5) << data[row + col * ROWS] << " ";
                index++;
            }
            std::cout << std::endl;
        }
    }

    // operator+
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::operator+(const MyType &other) const
    {
        MyType out;
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                out.data[index] = data[index] + other.data[index];
                index++;
            }
        }
        return out;
    }

    // operator+=
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline const Matrix<MatScalar, ROWS, COLS> &Matrix<MatScalar, ROWS, COLS>::operator+=(const MyType &other)
    {
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                data[index] = data[index] + other.data[index];
                index++;
            }
        }
        return *this;
    }

    // operator-
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::operator-(const MyType &other) const
    {
        MyType out;
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                out.data[index] = data[index] - other.data[index];
                index++;
            }
        }
        return out;
    }

    // operator-=
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline const Matrix<MatScalar, ROWS, COLS> &Matrix<MatScalar, ROWS, COLS>::operator-=(const MyType &other)
    {
        int index = 0;
        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {
                data[index] = data[index] - other.data[index];
                index++;
            }
        }
        return *this;
    }

    // operator*
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::operator*(const MyType &otherMat) const
    {
        EXPOP_MATRIX_ASSERT_STATIC(ROWS == COLS);

        MyType output(MatScalar(0));
        int index = 0;

        for(unsigned int col = 0; col < COLS; col++) {
            for(unsigned int row = 0; row < ROWS; row++) {

                for(unsigned int i = 0; i < ROWS; i++) {

                    output.data[index] += getConst(row, i) * otherMat.getConst(i, col);

                }

                index++;
            }
        }
        return output;
    }

    // multiply
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    template<unsigned int otherRows, unsigned int otherCols>
    inline Matrix<MatScalar, ROWS, otherCols> Matrix<MatScalar, ROWS, COLS>::multiply(
        const Matrix<MatScalar, otherRows, otherCols> &otherMat) const
    {
        // COLS and otherRows MUST be the same!
        EXPOP_MATRIX_ASSERT_STATIC(COLS == otherRows);

        Matrix<MatScalar, ROWS, otherCols> output(0.0f);

        int index = 0;

        for(unsigned int col = 0; col < otherCols; col++) {

            for(unsigned int row = 0; row < ROWS; row++) {

                // Using COLS and otherRows interchangeably here.
                for(unsigned int i = 0; i < COLS; i++) {

                    output.data[index] += getConst(row, i) * otherMat.getConst(i, col);

                }

                index++;
            }
        }

        return output;

    }

    // operator*(s)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::operator*(MatScalar s) const
    {
        MyType v;
        unsigned int arraySize = ROWS * COLS;

        for(unsigned int i = 0; i < arraySize; i++) {
            v.data[i] = data[i] * s;
        }
        return v;
    }

    // operator*=(s)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline const Matrix<MatScalar, ROWS, COLS> &Matrix<MatScalar, ROWS, COLS>::operator*=(MatScalar s)
    {
        unsigned int arraySize = ROWS * COLS;
        for(unsigned int i = 0; i < arraySize; i++) {
            data[i] = data[i] * s;
        }
        return *this;
    }

    // operator/(s)
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::operator/(MatScalar s) const
    {
        MyType v;
        unsigned int arraySize = ROWS * COLS;

        for(unsigned int i = 0; i < arraySize; i++) {
            v.data[i] = data[i] / s;
        }
        return v;
    }

    // magnitude
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar Matrix<MatScalar, ROWS, COLS>::magnitude(void) const
    {
        return std::sqrt(magnitudeSquared());
    }

    // magnitudeSquared
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar Matrix<MatScalar, ROWS, COLS>::magnitudeSquared(void) const
    {
        unsigned int arraySize = ROWS * COLS;
        MatScalar outVal(0);
        for(unsigned int i = 0; i < arraySize; i++) {
            outVal += data[i] * data[i];
        }

        return outVal;
    }

    // normalize
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::normalize(void) const
    {
        return (*this) / magnitude();
    }

    // dot
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline MatScalar Matrix<MatScalar, ROWS, COLS>::dot(const MyType &otherMat) const
    {
        unsigned int arraySize = ROWS * COLS;
        MatScalar outVal(0);

        for(unsigned int i = 0; i < arraySize; i++) {
            outVal += data[i] * otherMat.data[i];
        }
        return outVal;
    }

    // isZero
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline bool Matrix<MatScalar, ROWS, COLS>::isZero(void) const
    {
        unsigned int arraySize = ROWS * COLS;
        MatScalar zeroTest(0);

        for(unsigned int i = 0; i < arraySize; i++) {
            if(data[i] != zeroTest) return false;
        }
        return true;
    }

    // operator==
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline bool Matrix<MatScalar, ROWS, COLS>::operator==(const MyType &otherMat) const
    {
        unsigned int arraySize = ROWS * COLS;

        for(unsigned int i = 0; i < arraySize; i++) {
            if(data[i] != otherMat.data[i]) return false;
        }
        return true;
    }

    // operator!=
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline bool Matrix<MatScalar, ROWS, COLS>::operator!=(const MyType &otherMat) const
    {
        return !(operator==(otherMat));
    }

    // crossProduct
    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline Matrix<MatScalar, ROWS, COLS> Matrix<MatScalar, ROWS, COLS>::crossProduct(const MyType &v) const
    {
        // 3D Vectors only.
        EXPOP_MATRIX_ASSERT_STATIC(ROWS * COLS == 3);

        MyType out;
        out.data[0] =  data[1]   * v.data[2] - data[2]   *  v.data[1];
        out.data[1] = -v.data[2] * data[0]   - v.data[0] * -data[2];
        out.data[2] =  data[0]   * v.data[1] - data[1]   *  v.data[0];

        return out;
    }

    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline size_t Matrix<MatScalar, ROWS, COLS>::getNumRows()
    {
        return ROWS;
    }

    template<typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline size_t Matrix<MatScalar, ROWS, COLS>::getNumCols()
    {
        return COLS;
    }

    // Utility/helper functions...

    inline FMatrix4x4 makePerspectiveMatrix(
        float fovY,
        float aspectRatio,
        float zNear,
        float zFar)
    {
        fovY *= 3.14159 / 360.0;

        float tmp1 = std::cos(fovY) / std::sin(fovY);
        float tmp2 = zFar - zNear;

        FMatrix4x4 mat;

        mat(0, 0) = tmp1 / aspectRatio;
        mat(1, 1) = tmp1;
        mat(2, 2) = -(zFar + zNear) / tmp2;
        mat(2, 3) = -1;
        mat(3, 2) = (-2 * zNear * zFar) / tmp2;
        mat(3, 3) = 0;

        return mat;

    }

    inline FMatrix4x4 makeFrustumMatrix(
        float left,
        float right,
        float bottom,
        float top,
        float nearDist,
        float farDist)
    {
        FMatrix4x4 mat;

        mat.get(0, 0) =
            (2.0 * nearDist) /
            (right - left);

        mat(0, 2) = (right + left) / (right - left);
        mat(1, 1) = (2.0 * nearDist) / (top - bottom);
        mat(1, 2) = (top + bottom) / (top - bottom);
        mat(2, 2) = -(farDist + nearDist) / (farDist - nearDist);
        mat(2, 3) = -(2.0 * farDist * nearDist) / (farDist - nearDist);
        mat(3, 2) = -1;
        mat(3, 3) = 0;

        return mat;
    }

    inline FMatrix4x4 makeTranslationMatrix(const FVec3 &t)
    {
        FMatrix4x4 mat;
        mat(0, 3) = t.data[0];
        mat(1, 3) = t.data[1];
        mat(2, 3) = t.data[2];
        return mat;
    }

    inline FMatrix4x4 makeRotationMatrix(const FVec3 &axis, float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1 - c;

        FMatrix4x4 mat;

        mat(0, 0) = t*axis.data[0]*axis.data[0] + c;
        mat(0, 1) = t*axis.data[0]*axis.data[1] + s*axis.data[2];
        mat(0, 2) = t*axis.data[0]*axis.data[2] - s*axis.data[1];

        mat(1, 0) = t*axis.data[1]*axis.data[0] - s*axis.data[2];
        mat(1, 1) = t*axis.data[1]*axis.data[1] + c;
        mat(1, 2) = t*axis.data[1]*axis.data[2] + s*axis.data[0];

        mat(2, 0) = t*axis.data[2]*axis.data[0] + s*axis.data[1];
        mat(2, 1) = t*axis.data[2]*axis.data[1] - s*axis.data[0];
        mat(2, 2) = t*axis.data[2]*axis.data[2] + c;

        return mat;
    }

    inline FMatrix3x3 make2DRotationMatrix(float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);

        FMatrix3x3 mat;

        mat(0, 0) = c;
        mat(0, 1) = -s;
        mat(0, 2) = 0;

        mat(1, 0) = s;
        mat(1, 1) = c;
        mat(1, 2) = 0;

        return mat;
    }

    inline FMatrix4x4 makeScaleMatrix(const FVec3 &scale)
    {
        FMatrix4x4 mat;

        mat(0, 0) = scale.data[0];
        mat(1, 1) = scale.data[1];
        mat(2, 2) = scale.data[2];

        return mat;
    }

    // std::ostream output.
    template <typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline std::ostream &operator<<(std::ostream &out, const Matrix<MatScalar, ROWS, COLS> &mat)
    {
        unsigned int arraySize = ROWS * COLS;
        for(unsigned int i = 0; i < arraySize; i++) {
            out << mat.data[i];
            if(i != arraySize - 1) {
                out << " ";
            }
        }
        return out;
    }

    // std::istream input.
    template <typename MatScalar, unsigned int ROWS, unsigned int COLS>
    inline std::istream &operator>>(std::istream &in, Matrix<MatScalar, ROWS, COLS> &mat)
    {
        unsigned int arraySize = ROWS * COLS;
        for(unsigned int i = 0; i < arraySize; i++) {
            in >> mat.data[i];
        }
        return in;
    }

}


