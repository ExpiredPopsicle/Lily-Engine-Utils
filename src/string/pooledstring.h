#pragma once

#include <string>

namespace ExPop {

    class StringPool;

    /// PooledString is a single string inside of a StringPool. It
    /// isn't instantiated directly outside of a StringPool, but the
    /// reference type (PooledString::Ref) will end up all over the
    /// place. It's set up to automatically increment and decrement
    /// reference counts, and StringPool will delete a string when it
    /// is no longer referenced.
    class PooledString {
    public:

        /// Reference type. Can be directly typecasted to a
        /// std::string to dereference it. This is what StringPool
        /// will return when you ask it for a string. Make sure all of
        /// these are gone or clear()ed by the time you destroy the
        /// StringPool, otherwise it'll trigger an assert.
        class Ref {
        public:

            Ref(void);
            Ref(const Ref &ref);
            Ref(PooledString *str);
            ~Ref(void);
            Ref &operator=(const Ref &ref);

            /// Remove this reference and set the string pointer to
            /// NULL.
            void clear(void);

            /// Dereference this to a std::string.
            operator std::string(void) const;

            /// Returns true if this does not point to anything
            /// currently.
            bool isNull(void);

        private:

            void setRef(const Ref &ref);
            void removeRef(void);
            PooledString *realString;
        };

    private:

        // Private constructor here keeps us from accidentally getting
        // instantiated outside of a StringPool.
        PooledString(
            const std::string &str,
            StringPool *pool);

        unsigned int refCount;
        StringPool *pool;
        std::string str;

        // So that StringPool can call our private constructor.
        friend class StringPool;
    };

    /// StringPool stores PooledString objects.
    class StringPool {
    public:

        ~StringPool(void);

        /// Get a reference to a string from an std::string. If it
        /// does not exist in the string pool, it will be created and
        /// a reference to the new string will be returned.
        PooledString::Ref getOrAdd(const std::string &str);

    private:

        void removeString(const std::string &str);
        std::map<std::string, PooledString *> stringMap;

        // So that PooledString can call removeString when cleaning
        // up.
        friend class PooledString;
    };
}
