#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
using namespace std;
#endif /* __PROGTEST__ */

template<typename T>
class SPtr {
    private:
        class Counter {
            public:
                size_t counter = 0;
        };
        T * mPtr;
        Counter * mCnt;
    public:
        SPtr() : mPtr(nullptr), mCnt(new Counter) {}
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
            return *this;
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

template<typename T>
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
            delete [] mArray;
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
            return *this;
        }
        ~Vector() {
            delete [] mArray;
            mLen = 0;
            mCap = 0;
        }
        Vector & add(const T & item) {
            expandToLen(mLen);
            mArray[mLen++] = item;
            return *this;
        }
        Vector & add(T && item) {
            expandToLen(mLen);
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
        void clear() {
            delete mArray;
            mLen = 0;
            mCap = 0;
        }
        inline size_t size() const { return mLen; }
        inline size_t isEmpty() const { return mLen == 0; }
    private:
        void expandToLen(size_t len) {
            if (len >= mCap) {
                do {
                    mCap = mCap * 3 / 2 + 42;
                } while(len >= mCap);
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
                expandToLen(index + 1);
                mLen = index + 1;
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
    Vector<SPtr<Version>> mVersions;
    SPtr<Buffer> mCur = new Buffer, mLatest = nullptr;
    size_t mPos = 0;

    public:
    CFile(void) {}
    CFile(const CFile & other) {
        mPos = other.mPos;
        mVersions = other.mVersions;
        mCur = other.mCur;
        mLatest = other.mLatest;
    }
    CFile(CFile && other) {
        mPos = other.mPos;
        mVersions = other.mVersions;
        mCur = other.mCur;
        mLatest = other.mLatest;
        other.mPos = 0;
        other.mVersions.clear();
        other.mCur    = nullptr;
        other.mLatest = nullptr;
    }
    CFile & operator = (CFile other) {
        swap(mPos, other.mPos);
        swap(mVersions, other.mVersions);
        swap(mCur, other.mCur);
        swap(mLatest, other.mLatest);
        return *this;
    }
    ~CFile() {}

    bool seek(uint32_t offset) {
        if (offset > mCur -> size()) return false;
        mPos = offset;
        return true;
    }
    uint32_t read(uint8_t * dst, uint32_t bytes) {
        // cout << "Size: " << mCur -> size() << " x Pos: " << mPos + bytes << endl;
        if (mCur -> isEmpty()) return 0;
        size_t maxBound = min(mCur -> size(), mPos + bytes);
        size_t read = 0;
        for (; mPos < maxBound; mPos++, read++)
            dst[read] = (*mCur)[mPos];
        return read;
    }
    uint32_t write(const uint8_t * src, uint32_t bytes) {
        checkWrite();
        size_t end = mPos + bytes;
        for (size_t read = 0; mPos < end; mPos++, read++) {
            (*mCur)[mPos] = src[read];
        }
        return bytes;
    }
    void truncate(void) {
        checkWrite();
        mCur -> dropFrom(mPos);
    }
    uint32_t fileSize(void) const { return mCur -> size(); };
    void addVersion(void);
    bool undoVersion(void);
    void printBuffer(ostream & out) const {
        out << "Buf: Len: " << mCur -> size() << " Data: ";
        for (size_t i = 0; i < mCur -> size(); i++)
            out << (uint32_t)(*mCur)[i] << ", ";
        out << endl;
    }
    private:
    void checkWrite() {
        if (mCur.hasOne()) return;
        Buffer * copy = new Buffer;
        *copy = *mCur;
        mCur = SPtr(copy);
    }
};

#ifndef __PROGTEST__
bool writeTest(CFile & x, const initializer_list<uint8_t> & data, uint32_t wrLen) {
    return x.write(data.begin (), data.size ()) == wrLen;
}

bool readTest(CFile & x, const initializer_list<uint8_t> & data, uint32_t rdLen) {
    uint8_t tmp[100];
    uint32_t idx = 0;

    uint32_t len = x.read(tmp, rdLen);
    if(len != data.size ()) {
        cout << "Len mismatch: Ref(" << data.size() << ") x Me(" << len << ")" << endl;
        return false;
    }
    for(auto v : data)
        if(tmp[idx++] != v) {
            cout << "Data mismatch" << endl << "Ref: ";
            for (auto byte : data)
                cout << (uint32_t)byte << ", ";
            cout << endl << "Me:  ";
            for (size_t i = 0; i < len; i++) {
                auto byte = tmp[i];
                cout << (uint32_t)byte << ", ";
            }
            cout << endl;
            return false;
        }
    return true;
}

int main(void) {
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
#ifdef blbina
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
