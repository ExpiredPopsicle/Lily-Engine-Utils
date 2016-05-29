// ---------------------------------------------------------------------------
//
//   Lily Engine Utils
//
//   Copyright (c) 2012-2016 Clifford Jolly
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

#include <map>
#include <cassert>
#include <iostream>

#include <lilyengine/pooledstring.h>
using namespace std;

namespace ExPop
{
    // ----------------------------------------------------------------------
    //  PooledString::Ref
    // ----------------------------------------------------------------------

    PooledString::Ref::Ref(void) {
        realString = NULL;
    }

    PooledString::Ref::Ref(const Ref &ref) {
        realString = NULL;
        setRef(ref);
    }

    PooledString::Ref::Ref(PooledString *str) {
        this->realString = str;
        realString->refCount++;
    }

    PooledString::Ref::~Ref(void) {
        removeRef();
    }

    void PooledString::Ref::removeRef(void) {

        if(realString) {
            assert(realString->refCount);
            realString->refCount--;

            if(!realString->refCount) {
                // Last reference. Purge it from the pool.
                realString->pool->removeString(realString->str);
            }
        }

        realString = NULL;
    }

    void PooledString::Ref::setRef(const Ref &ref) {
        assert(!realString);
        realString = ref.realString;
        realString->refCount++;
        assert(realString->refCount);
    }

    PooledString::Ref &PooledString::Ref::operator=(const Ref &ref) {
        if(ref.realString != realString) {
            removeRef();
            setRef(ref);
        }
        return *this;
    }

    PooledString::Ref::operator std::string(void) const {
        if(!realString) return "null";
        return realString->str;
    }

    void PooledString::Ref::clear(void) {
        removeRef();
    }

    bool PooledString::Ref::isNull(void) {
        return !realString;
    }

    // ----------------------------------------------------------------------
    //  PooledString
    // ----------------------------------------------------------------------

    PooledString::PooledString(
        const std::string &str,
        StringPool *pool) {

        this->str = str;
        this->pool = pool;
        refCount = 0;
    }

    // ----------------------------------------------------------------------
    //  StringPool
    // ----------------------------------------------------------------------

    StringPool::~StringPool(void) {

        for(std::map<std::string, PooledString *>::iterator i = stringMap.begin(); i != stringMap.end(); i++) {
            cerr << "StringMap deletion with string referenced: " << (*i).first << endl;
        }

        assert(!stringMap.size());
    }

    PooledString::Ref StringPool::getOrAdd(const std::string &str) {
        PooledString *pooledStr = stringMap[str];
        if(!pooledStr) {
            pooledStr = new PooledString(str, this);
            stringMap[str] = pooledStr;
        }
        return PooledString::Ref(pooledStr);
    }

    void StringPool::removeString(const std::string &str) {
        std::string localString = str;
        PooledString *pooledStr = stringMap[str];
        assert(pooledStr);
        assert(!pooledStr->refCount);
        delete pooledStr;
        stringMap.erase(localString);
    }
}

