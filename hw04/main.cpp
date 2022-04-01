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
class SPointer {
    private:
        class Counter {
            public:
                size_t counter = 0;
        };
        T * mPtr;
        Counter * mCnt;
        SPointer(T * ptr) : mPtr(ptr), mCnt(new Counter) {
            mCnt -> counter ++;
        }
        SPointer(const SPointer & src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            mCnt -> counter ++;
        }
        SPointer(SPointer && src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            src.mCnt = nullptr;
            src.mPtr = nullptr;
        }
        SPointer & operator = (SPointer src) {
            swap(mPtr, src.mPtr);
            swap(mCnt, src.mCnt);
            /*{
                T * tmp = mPtr;
                mPtr = src.mPtr;
                src.mPtr = tmp;
            } {
                Counter * tmp = mCnt;
                mCnt = src.mCnt;
                src.mCnt = tmp;
            }*/

        }
        T & operator *  () const { return *mPtr; }
        T * operator -> () const { return mPtr; }
        bool operator == (const SPointer & other) const {
            return mPtr == other.mPtr;
        }
        bool operator != (const SPointer & other) const {
            return !(mPtr == other);
        }
        bool operator == (T * other) const {
            return mPtr == other;
        }
        bool operator != (T * other) const {
            return !(mPtr == other);
        }
        ~SPointer() {
            mCnt -> counter --;
            if (mCnt -> counter == 0) {
                delete mCnt;
                delete mPtr;
            }
            mCnt = nullptr;
            mPtr = nullptr;
        }
};

class CFile {
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
