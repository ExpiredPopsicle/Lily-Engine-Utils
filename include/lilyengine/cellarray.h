#pragma once

namespace ExPop {

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
        void set(const T &t, int x, int y);

        /// Get an element. Expands array if necessary.
        T &get(int x, int y);

        /// Get an element. Returns NULL if it's out of bounds.
        /// Doesn't expand the array.
        const T *getConst(int x, int y);

        /// Expand the array to hold a given element. Called
        /// internally by get and set. Slots are filled with whatever
        /// comes out of a default constructor for the data type.
        void expandToFit(int x, int y);

        /// Get the minimum X value.
        int getMinX(void) const;

        /// Get the maximum X value.
        int getMaxX(void) const;

        /// Get the minimum Y value.
        int getMinY(void) const;

        /// Get the maximum X value.
        int getMaxY(void) const;

        /// Get the width. This does not necessarily match the value
        /// returned from getMaxX().
        unsigned int getWidth(void) const;

        /// Get the height. This does not necessarily match the value
        /// returned from getMaxY().
        unsigned int getHeight(void) const;

    private:

        int width;
        int height;
        int offsetx;
        int offsety;
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
    void CellArray<T>::expandToFit(int x, int y) {

        // We don't necessarily need the origin to be in the range of
        // stuff we get, so the first time, just make sure that the
        // offset is set right where our first value is.
        if(!cells) {
            offsetx = x;
            offsety = y;
        }

        // First see if we have to expand this thing, and by how much.
        int expandleft = 0;
        int expandright = 0;
        int expandup = 0;
        int expanddown = 0;

        if(x < offsetx) {
            expandleft = offsetx - x;
        }
        if(x >= (offsetx + width)) {
            expandright = (x + 1) - (offsetx + width);
        }
        if(y < offsety) {
            expandup = offsety - y;
        }
        if(y >= (offsety + height)) {
            expanddown = (y + 1) - (offsety + height);
        }

        // Expand if necessary.
        if(expanddown || expandup || expandright || expandleft) {

            int newWidth = expandright + expandleft + width;
            int newHeight = expandup + expanddown + height;
            T *newCells = new T[newWidth * newHeight]();

            // Copy over the old data.
            for(int x1 = 0; x1 < width; x1++) {
                for(int y1 = 0; y1 < height; y1++) {
                    newCells[
                        (x1 + expandleft) + (y1 + expandup) * newWidth].tmp =
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
    void CellArray<T>::set(const T &t, int x, int y) {

        expandToFit(x, y);

        // Finally, set the thing we were going to set originally.
        int realx = x - offsetx;
        int realy = y - offsety;
        cells[realx + realy * width] = t;
    }

    template<typename T>
    T &CellArray<T>::get(int x, int y) {
        if(x < offsetx || x >= offsetx + width ||
           y < offsety || y >= offsety + height) {
            expandToFit(x, y);
        }
        return cells[(x-offsetx) + (y-offsety) * width];
    }

    template<typename T>
    const T *CellArray<T>::getConst(int x, int y) {
        if(x < offsetx || x >= offsetx + width ||
           y < offsety || y >= offsety + height) {
            return NULL;
        }
        return &(cells[(x-offsetx) + (y-offsety) * width]);
    }

    template<typename T>
    unsigned int CellArray<T>::getWidth(void) const {
        return width;
    }

    template<typename T>
    unsigned int CellArray<T>::getHeight(void) const {
        return height;
    }

    template<typename T>
    int CellArray<T>::getMinX(void) const {
        return offsetx;
    }

    template<typename T>
    int CellArray<T>::getMaxX(void) const {
        return offsetx + width;
    }

    template<typename T>
    int CellArray<T>::getMinY(void) const {
        return offsety;
    }

    template<typename T>
    int CellArray<T>::getMaxY(void) const {
        return offsety + height;
    }
}


