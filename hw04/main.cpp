#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
using namespace std;
#endif /* __PROGTEST__ */

template<class T>
class SPtr {
    private:
        class Counter {
            public:
                size_t counter = 0;
        };
        T * mPtr;
        Counter * mCnt;
    public:
        SPtr(T * ptr) : mPtr(ptr), mCnt(new Counter) {
            mCnt -> counter ++;
        }
        SPtr(const SPtr & src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            mCnt -> counter ++;
        }
        SPtr(SPtr && src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            src.mCnt = nullptr;
            src.mPtr = nullptr;
        }
        SPtr & operator = (SPtr src) {
            std::swap(mPtr, src.mPtr);
            std::swap(mCnt, src.mCnt);
        }
        T & operator *  () const { return *mPtr; }
        T * operator -> () const { return mPtr; }
        bool operator == (const SPtr & other) const {
            return mPtr == other.mPtr;
        }
        bool operator != (const SPtr & other) const {
            return !(mPtr == other);
        }
        bool operator == (T * other) const {
            return mPtr == other;
        }
        bool operator != (T * other) const {
            return !(mPtr == other);
        }
        bool hasOne() const { return mCnt -> counter == 1; }
        ~SPtr() {
            mCnt -> counter --;
            if (mCnt -> counter == 0) {
                delete mCnt;
                delete mPtr;
            }
            mCnt = nullptr;
            mPtr = nullptr;
        }
};

template<class T>
class Vector {
    private:
        T * mArray = nullptr;
        size_t mLen = 0, mCap = 0;
    public:
        Vector() {}
        Vector(const Vector & other) {
            mArray = new T[other.mLen];
            mLen = other.mLen;
            mCap = other.mLen;
            for (size_t i = 0; i < other.mLen; i++)
                mArray[i] = other.mArray[i];
        }
        Vector(Vector && other) {
            delete mArray;
            mArray = other.mArray;
            mLen = other.mLen;
            mCap = other.mCap;
            other.mArray = nullptr;
            other.mLen = 0;
            other.mCap = 0;
        }
        Vector & operator = (Vector other) {
            std::swap(mArray, other.mArray);
            std::swap(mCap,   other.mCap);
            std::swap(mLen,   other.mLen);
        }
        ~Vector() {
            delete mArray;
            mLen = 0;
            mCap = 0;
        }
        Vector & add(const T & item) {
            checkSizeBeforeInsert();
            mArray[mLen++] = item;
            return *this;
        }
        Vector & add(T && item) {
            checkSizeBeforeInsert();
            mArray[mLen++] = item;
            return *this;
        }
        Vector & dropFrom(size_t index) {
             mLen = index;
             return *this;
        }
        T & get(const size_t index) {
            workAferEnd(index);
            return mArray[index];
        }
        const T & get(const size_t index) const {
            workAferEnd(index);
            return mArray[index];
        }
        T & operator[](const size_t index) {
            workAferEnd(index);
            return mArray[index];
        }
        const T & operator[](const size_t index) const {
            workAferEnd(index);
            return mArray[index];
        }
        void prepare(const size_t newCapacity) {
            if (newCapacity <= mCap) return;
            mCap = newCapacity;
            applyNewCapacity();
        }
        size_t size() const { return mLen; }
        size_t isEmpty() const { return mLen == 0; }
    private:
        void checkSizeBeforeInsert() {
            if (mLen >= mCap) {
                do {
                    mCap = mCap * 3 / 2 + 42;
                } while(mLen >= mCap);
                applyNewCapacity();
            }
        }
        void applyNewCapacity() {
            T * newArray = new T[mCap];
            for (size_t i = 0; i < mLen; i++)
                newArray[i] = move(mArray[i]);
            delete [] mArray;
            mArray = newArray;
        }
        void workAferEnd(const size_t index) {
            if (index >= mLen) {
                mLen = index + 1;
                checkSizeBeforeInsert();
            }
        }
};

typedef Vector<uint8_t> Buffer;

class Version {
    class Change {
        size_t mIndex;
        Buffer mBuffer;
    };
    size_t mPos;
    size_t mLen;
    Vector<Change> mChanges;
};

class CFile {
    Vector<SPtr<Version>> versions;
    SPtr<Buffer> cur, latest = nullptr;
    size_t mPos;

    public:
        CFile(void);
        // copy cons, dtor, op=
        bool seek(uint32_t offset);
        uint32_t read(uint8_t * dst, uint32_t bytes);
        uint32_t write(const uint8_t * src, uint32_t bytes);
        void truncate(void);
        uint32_t fileSize(void) const;
        void addVersion(void);
        bool undoVersion(void);
    private:
        // todo
};

#ifndef __PROGTEST__
#ifdef blbina
bool writeTest(CFile & x, const initializer_list<uint8_t> & data, uint32_t wrLen) {
    return x.write(data.begin (), data.size ()) == wrLen;
}

bool readTest(CFile & x, const initializer_list<uint8_t> & data, uint32_t rdLen) {
    uint8_t tmp[100];
    uint32_t idx = 0;

    if(x.read(tmp, rdLen) != data.size ())
        return false;
    for(auto v : data)
        if(tmp[idx++] != v)
            return false;
    return true;
}
#endif

int main(void) {
#ifdef blbina
    CFile f0;

    assert( writeTest(f0, { 10, 20, 30 }, 3));
    assert( f0.fileSize () == 3);
    assert( writeTest(f0, { 60, 70, 80 }, 3));
    assert( f0.fileSize () == 6);
    assert( f0.seek(2));
    assert( writeTest(f0, { 5, 4 }, 2));
    assert( f0.fileSize () == 6);
    assert( f0.seek(1));
    assert( readTest(f0, { 20, 5, 4, 70, 80 }, 7));
    assert( f0.seek(3));
    f0.addVersion();
    assert( f0.seek(6));
    assert( writeTest(f0, { 100, 101, 102, 103 }, 4));
    f0.addVersion();
    assert( f0.seek(5));
    CFile f1(f0);
    f0.truncate ();
    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70 }, 20));
    assert( f0.undoVersion ());
    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70, 80, 100, 101, 102, 103 }, 20));
    assert( f0.undoVersion ());
    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70, 80 }, 20));
    assert( !f0.seek(100));
    assert( writeTest(f1, { 200, 210, 220 }, 3));
    assert( f1.seek(0));
    assert( readTest(f1, { 10, 20, 5, 4, 70, 200, 210, 220, 102, 103 }, 20));
    assert( f1.undoVersion ());
    assert( f1.undoVersion ());
    assert( readTest(f1, { 4, 70, 80 }, 20));
    assert( !f1.undoVersion ());
#endif

    cout << "All tests padded!" << endl;
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
