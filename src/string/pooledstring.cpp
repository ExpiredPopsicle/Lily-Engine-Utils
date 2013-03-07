#include <map>
#include <cassert>
#include <iostream>

#include "pooledstring.h"
using namespace std;

namespace ExPop {

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

