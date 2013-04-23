#pragma once

#include <cassert>

namespace ExPop {

    typedef ptrdiff_t CAINDEXTYPE;
    typedef size_t CAUNSIGNED;

    /// 2D auto-expanding array with integer indices. Supports
    /// negative indicies. Will reallocate as necessary. Things in
    /// newly allocated space will be default-initialized. Things that
    /// were previously defined will then be copied over those using
    /// operator=.
    template<typename T>
    class CellArray {
    public:

        CellArray(void);
        ~CellArray(void);

        /// Set an element. Expands array if necessary.
        void set(const T &t, CAINDEXTYPE x, CAINDEXTYPE y);

        /// Get an element. Expands array if necessary.
        T &get(CAINDEXTYPE x, CAINDEXTYPE y);

        /// Get an element. Returns NULL if it's out of bounds.
        /// Doesn't expand the array.
        const T *getConst(CAINDEXTYPE x, CAINDEXTYPE y);

        /// Expand the array to hold a given element. Called
        /// internally by get and set. Slots are filled with whatever
        /// comes out of a default constructor for the data type.
        void expandToFit(CAINDEXTYPE x, CAINDEXTYPE y);

        /// Get the minimum X value.
        CAINDEXTYPE getMinX(void) const;

        /// Get the maximum X value.
        CAINDEXTYPE getMaxX(void) const;

        /// Get the minimum Y value.
        CAINDEXTYPE getMinY(void) const;

        /// Get the maximum X value.
        CAINDEXTYPE getMaxY(void) const;

        /// Get the width. This does not necessarily match the value
        /// returned from getMaxX().
        CAUNSIGNED getWidth(void) const;

        /// Get the height. This does not necessarily match the value
        /// returned from getMaxY().
        CAUNSIGNED getHeight(void) const;

        /// Delete everything and reset to the default state.
        void clear(void);

    private:

        CAUNSIGNED width;
        CAUNSIGNED height;
        CAINDEXTYPE offsetx;
        CAINDEXTYPE offsety;
        T *cells;

    };

    template<typename T>
    CellArray<T>::CellArray(void) {
        cells = NULL;
        width = 0;
        height = 0;
        offsetx = 0;
        offsety = 0;
    }

    template<typename T>
    CellArray<T>::~CellArray(void) {
        if(cells) {
            delete[] cells;
        }
    }

    template<typename T>
    void CellArray<T>::expandToFit(CAINDEXTYPE x, CAINDEXTYPE y) {

        // We don't necessarily need the origin to be in the range of
        // stuff we get, so the first time, just make sure that the
        // offset is set right where our first value is.
        if(!cells) {
            offsetx = x;
            offsety = y;
        }

        // First see if we have to expand this thing, and by how much.
        CAINDEXTYPE expandleft = 0;
        CAINDEXTYPE expandright = 0;
        CAINDEXTYPE expandup = 0;
        CAINDEXTYPE expanddown = 0;

        if(x < offsetx) {
            expandleft = offsetx - x;
        }
        if(x >= (CAINDEXTYPE)(offsetx + width)) {
            expandright = (x + 1) - (CAINDEXTYPE)(offsetx + width);
        }
        if(y < offsety) {
            expandup = offsety - y;
        }
        if(y >= (CAINDEXTYPE)(offsety + height)) {
            expanddown = (y + 1) - (CAINDEXTYPE)(offsety + height);
        }

        // Expand if necessary.
        if(expanddown || expandup || expandright || expandleft) {

            CAUNSIGNED newWidth = expandright + expandleft + width;
            CAUNSIGNED newHeight = expandup + expanddown + height;

            // If we've wrapped around, just clamp width and height.
            // FIXME: Might be specifying something here that's fewer
            // bits than the actual address space.
            if(newWidth < width) {
                newWidth = ~0;
            }
            if(newHeight < height) {
                newHeight = ~0;
            }

            // This might throw an exception. Thankfully we haven't
            // modified anything yet.
            T *newCells = new T[newWidth * newHeight]();

            assert(newCells);

            // Copy over the old data.
            for(CAUNSIGNED x1 = 0; x1 < width; x1++) {
                for(CAUNSIGNED y1 = 0; y1 < height; y1++) {

                    CAUNSIGNED srcx = (x1 + expandleft) % ((CAUNSIGNED)~0);
                    CAUNSIGNED srcy = (y1 + expandup) % ((CAUNSIGNED)~0);

                    newCells[
                        srcx + srcy * newWidth].tmp =
                        cells[x1 + y1 * width].tmp;
                }
            }

            delete[] cells;
            cells = newCells;
            width = newWidth;
            height = newHeight;
            offsetx -= expandleft;
            offsety -= expandup;
        }
    }

    template<typename T>
    void CellArray<T>::set(const T &t, CAINDEXTYPE x, CAINDEXTYPE y) {

        get(x, y) = t;

    }

    template<typename T>
    T &CellArray<T>::get(CAINDEXTYPE x, CAINDEXTYPE y) {

        expandToFit(x, y);
        CAUNSIGNED srcx = (x - offsetx) % ((CAUNSIGNED)~0);
        CAUNSIGNED srcy = (y - offsety) % ((CAUNSIGNED)~0);
        return cells[srcx + srcy * width];
    }

    template<typename T>
    const T *CellArray<T>::getConst(CAINDEXTYPE x, CAINDEXTYPE y) {

        if(x < offsetx ||
           x >= (CAINDEXTYPE)(offsetx + width) ||
           y < offsety ||
           y >= (CAINDEXTYPE)(offsety + height)) {
            return NULL;
        }

        CAUNSIGNED srcx = (x - offsetx) % ((CAUNSIGNED)~0);
        CAUNSIGNED srcy = (y - offsety) % ((CAUNSIGNED)~0);

        return cells[srcx + srcy * width];
    }

    template<typename T>
    CAUNSIGNED CellArray<T>::getWidth(void) const {
        return width;
    }

    template<typename T>
    CAUNSIGNED CellArray<T>::getHeight(void) const {
        return height;
    }

    template<typename T>
    CAINDEXTYPE CellArray<T>::getMinX(void) const {
        return offsetx;
    }

    template<typename T>
    CAINDEXTYPE CellArray<T>::getMaxX(void) const {
        return offsetx + width;
    }

    template<typename T>
    CAINDEXTYPE CellArray<T>::getMinY(void) const {
        return offsety;
    }

    template<typename T>
    CAINDEXTYPE CellArray<T>::getMaxY(void) const {
        return offsety + height;
    }

    template<typename T>
    void CellArray<T>::clear(void) {
        delete cells;
        cells = NULL;
        width = 0;
        height = 0;
        offsetx = 0;
        offsety = 0;
    }

}


