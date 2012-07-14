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

#pragma once

#include <vector>

namespace ExPop {

    typedef unsigned int HashValue;

    // Some simple hash functions. These will be plugged into the hash
    // table as a default parameter if the default constructor is used
    // on the hash table. Note that these hash functions are NOT
    // cryptographically secure in any way. They're just there to
    // split up data in the hash table.

    /// String hash.
    inline HashValue genericHashFunc(const std::string &str) {
        HashValue hash = 0;
        for(unsigned int i = 0; i < str.size(); i++) {
            hash = str[i] + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    /// Given that HashValues are really just unsigned ints, this just
    /// returns the unsigned int.
    inline HashValue genericHashFunc(const unsigned int &i) {
        return i;
    }

    /// A hash table implementation with template stuff. Note that
    /// there's one big issue (or a few) with this implementation in
    /// that it keeps instances of the ValueType even for empty
    /// slots. These are initialized with some a constructor and are
    /// set back to that (with operator=) when the slot is
    /// cleared. Keep this in mind if you ever make a hash table of
    /// something other than a standard data type or a pointer.
    ///
    /// The idea with this class was to have an interface similar to
    /// std::map.
    ///
    /// KeyType and ValueType must have copy constructors and
    /// operator=() defined. KeyType must also have operator<()
    /// defined.
    template<class KeyType, class ValueType>
    class HashTable {
    public:

        /// Hash function for whatever data type we're using.
        typedef HashValue (*HashFunction)(
            const KeyType &t);

        /// Function type to compare two keys. Should return -1, 0, or
        /// 1. (0 being equal. Like strcmp.)
        typedef int (*KeyCompareFunction)(
            const KeyType &t1,
            const KeyType &t2);

        /// Initialize a hash table with a given number of buckets and
        /// slots. The number of slots will grow as new data is added
        /// that exceeds is capacity, but the number of buckets will
        /// not.
        ///
        /// NOTE: This will create as many instances of the value
        /// class as there are slots, and it will initialize them to
        /// their default constructor!
        HashTable(
            unsigned int slots = 256,
            unsigned int buckets = 16,
            HashFunction hashFunc = genericHashFunc);

        /// Initialize by copying from from another hash table.
        HashTable(const HashTable &otherTable);

        /// Destructor. When cleaning up, a lot of destructors for the
        /// key class will be called.
        ~HashTable(void);

        /// Copy all the data from another table.
        void copyFromOtherTable(
            const HashTable &otherTable,
            bool overwrite = true);

        /// This just copies over the contents of another hash
        /// table. It's probably really slow, too.
        HashTable<KeyType, ValueType> &operator=(
            const HashTable &otherTable);

        /// Set a value directly. Copies key (which must have
        /// operator=() and a copy constructor).
        bool setValue(
            const KeyType &key,
            const ValueType &val,
            bool overwrite,
            bool clearSlot);

        /// Get a value and copy it to outVal. Returns true if it
        /// succeeds. This is useful if you want to get something but
        /// you don't know if it's there or not. Using operator[] will
        /// create a new instance in the table, but asking for the
        /// count() will cause multiple table lookups. This has the
        /// disadvantage of copying into outVal instead of just
        /// returning a reference. Pick your poison for accessing data
        /// in the map.
        bool getValue(
            const KeyType &key,
            ValueType *outVal);

        /// Clear a key. (Causes key destructor to fire.)
        void erase(const KeyType &key);

        /// Get a value. Note that the reference it returns here won't
        /// be valid after reallocating. Copies key (which must have
        /// operator=() and a copy constructor).
        ValueType &operator[](const KeyType &key);

        /// Dump out a list of keys.
        void dumpKeys(std::vector<KeyType> &keys) const;

        /// Get the number of things in the whole map.
        unsigned int size(void) const;

        /// See if a key is present without actually modifying
        /// anything.
        unsigned int count(const KeyType &key) const;

        /// Clear out anything in the hash table.
        void clear(void);

    private:

        HashFunction hashFunc;

        /// Single slot in a bucket.
        class HashTableSlot {
        public:
            HashTableSlot *next;
            HashValue hashVal;
            KeyType key;
            ValueType value;
        };

        /// Single bucket. Hash value to get to this is determined by
        /// its position allBuckets.
        class HashTableBucket {
        public:
            HashTableSlot *slots;

            HashTableBucket(void) {
                slots = NULL;
            }
        };

        /// All of the buckets as an array.
        HashTableBucket *allBuckets;
        unsigned int numBuckets;

        /// All of the slots as one array Allocated as a block in an
        /// attempt to have at least some cache locality when doing
        /// lookups. Probably fails on larger hash maps.
        HashTableSlot *allSlots;
        unsigned int numSlots;

        /// A list of free slots as pointers to slots.
        HashTableSlot **freeSlots;
        unsigned int numFreeSlots;

        /// Free a slot. Just set the value back to the default
        /// constructor and return it to the free list. Doesn't do
        /// anything with links.
        void freeASlot(HashTableSlot *slot);

        /// Find a free slot. Expand allocated area if there isn't
        /// enough room.
        HashTableSlot *getAFreeSlot(
            HashValue hash,
            const KeyType &key,
            const ValueType &val,
            HashTableSlot ***lastBackPointer);

        /// Find a slot in the hash table by key. Keep a pointer to
        /// the last pointer so we know how to insert before whatever
        /// we end up hitting.
        HashTableSlot *findSlot(
            const KeyType &key,
            HashTableSlot ***prevBackPointer,
            bool *exactMatchFound,
            HashValue *hashValueReturn = NULL) const;

        /// Allocate memory and set up buckets and slots.
        void init(
            unsigned int slots,
            unsigned int buckets,
            HashFunction hashFunc);

        /// Clean up.
        void deinit(void);

        /// This sets a value in a slot, possibly creating a new slot
        /// to do it, or deleting an occupied slot. It's used
        /// internally, once a lot of the data is already available,
        /// so excuse the huge parameter list. val can only be NULL if
        /// clearSlot is true. It's ugly and it's private for a
        /// reason. :P
        bool setValueSlot(
            HashTableSlot *currentSlot,
            HashTableSlot **lastBackPointer,
            bool overwrite,
            bool clearSlot,
            HashValue hash,
            const KeyType &key,
            const ValueType *val,
            bool matchFound,
            HashTableSlot **finalOutputSlot = NULL);

    };

    // ----------------------------------------------------------------------
    //   HashTable implementation
    // ----------------------------------------------------------------------

    template<class KeyType, class ValueType>
    HashTable<KeyType, ValueType>::HashTable(unsigned int slots, unsigned int buckets,
              HashFunction hashFunc) {

        init(slots, buckets, hashFunc);
    }

    template<class KeyType, class ValueType>
    HashTable<KeyType, ValueType>::HashTable(const HashTable &otherTable) {

        // Copy initial sizes from the other table.
        init(otherTable->numSlots,
             otherTable->numBuckets,
             otherTable->hashFunc);

        // Then copy all the data in.
        copyFromOtherTable(otherTable);
    }

    template<class KeyType, class ValueType>
    HashTable<KeyType, ValueType>::~HashTable(void) {
        deinit();
    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::copyFromOtherTable(
        const HashTable &otherTable, bool overwrite) {

        // FIXME: There's definitely faster ways to do this. But this
        // was the quick to-implement-way.

        std::vector<KeyType> allKeys;
        otherTable.dumpKeys(allKeys);

        for(unsigned int i = 0; i < allKeys.size(); i++) {
            setValue(
                allKeys[i], otherTable[allKeys[i]],
                overwrite, false);
        }

    }

    template<class KeyType, class ValueType>
    HashTable<KeyType, ValueType> &HashTable<KeyType, ValueType>::operator=(
        const HashTable &otherTable) {

        deinit();

        init(otherTable->numSlots,
             otherTable->numBuckets,
             otherTable->hashFunc);

        copyFromOtherTable(otherTable);

        return *this;
    }

    template<class KeyType, class ValueType>
    bool HashTable<KeyType, ValueType>::setValue(
        const KeyType &key,
        const ValueType &val,
        bool overwrite,
        bool clearSlot) {

        HashTableSlot **lastBackPointer = NULL;
        bool matchFound = false;
        HashValue hash = 0;

        HashTableSlot *currentSlot = findSlot(key, &lastBackPointer, &matchFound, &hash);

        return setValueSlot(
            currentSlot, lastBackPointer,
            overwrite, clearSlot,
            hash, key,
            &val, matchFound);

    }

    template<class KeyType, class ValueType>
    bool HashTable<KeyType, ValueType>::getValue(
        const KeyType &key,
        ValueType *outVal) {

        bool exactMatch = false;
        HashTableSlot *slot = findSlot(key, NULL, &exactMatch);
        if(exactMatch) {
            *outVal = slot->value;
            return true;
        }
        return false;

    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::erase(
        const KeyType &key) {

        HashTableSlot **lastBackPointer = NULL;
        bool matchFound = false;
        HashValue hash = 0;

        HashTableSlot *currentSlot = findSlot(key, &lastBackPointer, &matchFound, &hash);

        setValueSlot(
            currentSlot, lastBackPointer,
            true, true,
            hash, key,
            NULL, matchFound);
    }

    template<class KeyType, class ValueType>
    ValueType &HashTable<KeyType, ValueType>::operator[](
        const KeyType &key) {

        HashTableSlot **lastBackPointer = NULL;
        bool matchFound = false;
        HashValue hash = 0;

        HashTableSlot *currentSlot = findSlot(key, &lastBackPointer, &matchFound, &hash);

        if(matchFound) {
            // Found a match.
            return currentSlot->value;
        }

        // We didn't find a slot, so we'll have to make a new one.

        // Make something with whatever the default constructor
        // is.
        ValueType val;

        // Set that.
        HashTableSlot *outSlot = NULL;

        setValueSlot(
            currentSlot, lastBackPointer,
            false, false,
            hash, key,
            &val, matchFound,
            &outSlot);

        assert(outSlot);

        return outSlot->value;

    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::dumpKeys(
        std::vector<KeyType> &keys) const {

        for(unsigned int i = 0; i < numBuckets; i++) {
            for(HashTableSlot *slot = allBuckets[i].slots; slot; slot = slot->next) {

                keys.push_back(slot->key);
            }
        }
    }

    template<class KeyType, class ValueType>
    unsigned int HashTable<KeyType, ValueType>::size(void) const {
        return numSlots - numFreeSlots;
    }

    template<class KeyType, class ValueType>
    unsigned int HashTable<KeyType, ValueType>::count(
        const KeyType &key) const {

        bool matchFound = false;
        HashTableSlot *currentSlot = findSlot(key, NULL, &matchFound, NULL);
        return matchFound;
    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::freeASlot(
        HashTableSlot *slot) {

        freeSlots[numFreeSlots] = slot;

        // FIXME: This is probably a terrible way of resetting
        // values. But we need to clear values, otherwise we could
        // have things like lingering reference-counting objects
        // lying around in free slots.
        slot->value = ValueType();

        numFreeSlots++;
    }

    template<class KeyType, class ValueType>
    typename HashTable<KeyType, ValueType>::HashTableSlot *HashTable<KeyType, ValueType>::getAFreeSlot(
        HashValue hash,
        const KeyType &key,
        const ValueType &val,
        HashTable::HashTableSlot ***lastBackPointer) {

        if(!numFreeSlots) {

            // Ran out of room so we need to expand storage
            // here. Warning: Horrible, ugly, nasty pointer
            // arithmetic follows to fix up all the pointers in
            // each Slot.

            HashTableSlot *oldSlots = allSlots;
            HashTableSlot **oldFreeSlots = freeSlots;
            unsigned int oldNumSlots = numSlots;

            numSlots = numSlots * 2;
            allSlots = new HashTableSlot[numSlots];

            // Fix up pointers everywhere.
            for(unsigned int i = 0; i < oldNumSlots; i++) {

                allSlots[i] = oldSlots[i];
                if(oldSlots[i].next) {
                    allSlots[i].next = allSlots + (oldSlots[i].next - oldSlots);
                } else {
                    allSlots[i].next = NULL;
                }

                // If we run into the pointer we're holding onto as a
                // backpointer while we're doing this, make sure that
                // gets updated too.
                if(&(oldSlots[i].next) == *lastBackPointer) {
                    *lastBackPointer = &(allSlots[i].next);
                }

            }

            // Fix up bucket start pointers.
            for(unsigned int i = 0; i < numBuckets; i++) {
                if(allBuckets[i].slots) {
                    std::ptrdiff_t slotNum = allBuckets[i].slots - oldSlots;
                    allBuckets[i].slots = allSlots + slotNum;
                }
            }

            // Copy over the free slots list, fixing up pointers
            // as we go.
            freeSlots = new HashTableSlot*[numSlots];
            for(unsigned int i = 0; i < numFreeSlots; i++) {
                freeSlots[i] = allSlots + (oldFreeSlots[i] - oldSlots);
            }

            // Add all the newly created ones to the free list.
            for(unsigned int i = oldNumSlots; i < numSlots; i++) {
                freeSlots[numFreeSlots] = &(allSlots[i]);
                numFreeSlots++;
            }

            delete[] oldSlots;
            delete[] oldFreeSlots;
        }

        // Now just pull the last free slot off the end of the
        // list and use that.

        HashTableSlot *freeSlot = freeSlots[numFreeSlots - 1];
        numFreeSlots--;

        freeSlot->next = NULL;
        freeSlot->hashVal = hash;
        freeSlot->key = key;
        freeSlot->value = val;

        return freeSlot;
    }

    template<class KeyType, class ValueType>
    typename HashTable<KeyType, ValueType>::HashTableSlot *HashTable<KeyType, ValueType>::findSlot(
        const KeyType &key,
        HashTableSlot ***prevBackPointer,
        bool *exactMatchFound,
        HashValue *hashValueReturn) const {

        // Figure out which bucket we're going to stick this in.
        HashValue hash = hashFunc(key);

        // Save the hash.
        if(hashValueReturn) *hashValueReturn = hash;

        // Get the bucket.
        unsigned int bucketNum = hash % numBuckets;
        HashTableBucket *bucket = &(allBuckets[bucketNum]);

        *exactMatchFound = false;
        if(prevBackPointer) *prevBackPointer = &bucket->slots;

        // Search through the slots by hash first, then start
        // comparing actual key values.
        HashTableSlot *currentSlot = bucket->slots;
        while(currentSlot) {

            if(currentSlot->hashVal > hash) {

                // Put it before this one.
                break;
            }

            if(currentSlot->hashVal == hash) {

                // Hit a slot with the same hash. Keep going until
                // we find a slot that's either a higher hash or
                // something with the same or greater actual
                // value.

                while(currentSlot->hashVal == hash) {

                    // Probably fires off some operator<()
                    // function.
                    int comparison = (currentSlot->key < key);

                    if(comparison == 0) {
                        // Found an exact match. Done.
                        *exactMatchFound = true;
                        break;
                    }

                    if(comparison > 0) {
                        // Went beyond the right place. Done.
                        break;
                    }

                    // Iterate.
                    if(prevBackPointer) *prevBackPointer = &currentSlot->next;
                    currentSlot = currentSlot->next;

                    if(!currentSlot) {
                        // Reached the end of the list
                        // anyway. Done
                        break;
                    }
                }

                return currentSlot;
            }

            // Iterate.
            if(prevBackPointer) *prevBackPointer = &currentSlot->next;
            currentSlot = currentSlot->next;
        }

        return currentSlot;

    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::init(
        unsigned int slots, unsigned int buckets,
        HashFunction hashFunc) {

        numBuckets = buckets;
        numSlots = slots;

        // Allocate all the slots and buckets in big chunks so we
        // have something resembling locality.
        allSlots = new HashTableSlot[slots];
        allBuckets = new HashTableBucket[buckets];

        // Initialize list of free slots.
        freeSlots = new HashTableSlot*[slots];
        for(unsigned int i = 0; i < slots; i++) {
            freeSlots[i] = &(allSlots[i]);
        }
        numFreeSlots = slots;

        this->hashFunc = hashFunc;
    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::deinit(void) {
        delete[] allSlots;
        delete[] allBuckets;
        delete[] freeSlots;
    }

    template<class KeyType, class ValueType>
    bool HashTable<KeyType, ValueType>::setValueSlot(
        HashTableSlot *currentSlot,
        HashTableSlot **lastBackPointer,
        bool overwrite,
        bool clearSlot,
        HashValue hash,
        const KeyType &key,
        const ValueType *val,
        bool matchFound,
        HashTableSlot **finalOutputSlot) {

        HashTableSlot *newSlot = NULL;

        if(!currentSlot) {

            // We made it to the end of the list without finding
            // an appropriate place to put this.

            if(clearSlot) return false;

            newSlot = getAFreeSlot(hash, key, *val,
                                   &lastBackPointer);

            if(!newSlot) {

                // Out of memory. (And we're set to not expand.)
                return false;
            }

            *lastBackPointer = newSlot;

        } else {

            if(matchFound) {

                // Had an exact collision.

                if(clearSlot) {

                    *lastBackPointer = currentSlot->next;

                    freeASlot(currentSlot);

                } else {

                    if(!overwrite) {
                        return false;
                    }

                    newSlot = currentSlot;
                    newSlot->value = *val;
                }

            } else {

                // Found a place to put a new slot between two
                // slots.

                if(clearSlot) return false;

                newSlot = getAFreeSlot(hash, key, *val,
                                       &lastBackPointer);

                if(!newSlot) {
                    return false;
                }

                newSlot->next = *lastBackPointer;
                *lastBackPointer = newSlot;

            }
        }

        if(finalOutputSlot) {
            *finalOutputSlot = newSlot;
        }

        return true;
    }

    template<class KeyType, class ValueType>
    void HashTable<KeyType, ValueType>::clear(void) {

        // This is probably cheating, but whatever.
        deinit();
        init(numSlots,
             numBuckets,
             hashFunc);
    }


};

