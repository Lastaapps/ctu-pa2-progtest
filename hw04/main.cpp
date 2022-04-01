#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
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
        SPtr() : mPtr(nullptr), mCnt(new Counter) {
            mCnt -> counter ++;
        }
        SPtr(T * ptr) : mPtr(ptr), mCnt(new Counter) {
            mCnt -> counter ++;
            // cout << "Making ptr new  " << typeid(T).name() << endl;
        }
        SPtr(const SPtr & src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            mCnt -> counter ++;
            // cout << "Making ptr copy " << typeid(T).name() << endl;
        }
        SPtr(SPtr && src) {
            if (&src == this) return;
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            src.mCnt = nullptr;
            src.mPtr = nullptr;
            // cout << "Making ptr move " << typeid(T).name() << endl;
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
            mArray = new T [other.mLen];
            mLen = other.mLen;
            mCap = other.mLen;
            for (size_t i = 0; i < other.mLen; i++)
                mArray[i] = other.mArray[i];
        }
        Vector(Vector && other) {
            if (&other == this) return;
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
            assert(index < mLen);
            return mArray[index];
        }
        inline T & pop() {
            assert (mLen > 0);
            T & toReturn = get(lastIndex());
            mLen--;
            return toReturn;
        }
        inline T & operator[](const size_t index) { return get(index); }
        inline const T & operator[](const size_t index) const { return get(index); }

        void prepare(const size_t newCapacity) {
            if (newCapacity <= mCap) return;
            mCap = newCapacity;
            applyNewCapacity();
        }
        void setSize(const size_t newLen) {
            expandToLen(newLen);
            mLen = newLen;
        }
        void clear() {
            delete mArray;
            mLen = 0;
            mCap = 0;
        }
        size_t lastIndex() const { return mLen > 0 ? mLen - 1 : 0; }
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
            T * newArray = new T [mCap];
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
    public:
        struct Iterator {
            private:
                T * ptr;
            public:
                Iterator(T * p) : ptr(p) {}
                T operator * () const {
                    return *ptr;
                }
                Iterator& operator ++() {
                    ptr++;
                    return *this;
                }
                bool operator < (const Iterator & other) const {
                    return ptr < other.ptr;
                }
                bool operator ==(const Iterator & other) const {
                    return ptr == other.ptr;
                }
                bool operator !=(const Iterator & other) const {
                    return !(*this == other);
                };
        };
        Iterator begin() { return Iterator(mArray); }
        Iterator end() { return Iterator(mArray + mLen); }
};

typedef Vector<uint8_t> Buffer;

class Version {
    private:
        class Change {
            private:
                size_t mIndex;
                Buffer mBuffer;
            public:
                Change() : mIndex(0) {}
                Change(size_t index) : mIndex(index) {}
                friend class CFile;
        friend ostream & operator<<(ostream & out, Version & data);
        };
        size_t mPos;
        size_t mLen;
        Vector<Change> mChanges;
    public:
        Version(size_t pos, size_t len, Vector<Change> & changes)
            : mPos(pos), mLen(len), mChanges(changes) {}
        Version(size_t pos, size_t len, Vector<Change> && changes)
            : mPos(pos), mLen(len), mChanges(changes) {}
        friend class CFile;
        friend ostream & operator<<(ostream & out, Version & data);
};

ostream & operator<<(ostream & out, Version & data) {
    out << "Version - Pos: " << data.mPos
        << " Len: " << data.mLen << " changes: "
        << data.mChanges.size() << endl;
    for (Version::Change c : data.mChanges) {
        out << "- " << c.mIndex << ": ";
        for (const uint8_t byte : c.mBuffer)
            out << (uint32_t)byte << ", ";
        out << endl;
    }
    return out;
}

class CFile {
    size_t mPos = 0, mLatestPos = 0;
    Vector<SPtr<Version>> mVersions;
    SPtr<Buffer> mCur = new Buffer, mLatest = new Buffer;

    public:
    CFile(void) {}
    CFile(const CFile & other)
    : mPos(other.mPos), mLatestPos(other.mLatestPos),
      mVersions(other.mVersions),
      mCur(other.mCur), mLatest(other.mLatest) {}
    CFile(CFile && other)
    : mPos(other.mPos), mLatestPos(other.mLatestPos),
      mVersions(other.mVersions),
      mCur(other.mCur), mLatest(other.mLatest) {
        if (&other == this) return;
        other.mPos = 0;
        other.mLatestPos = 0;
        other.mVersions.clear();
        other.mCur    = nullptr;
        other.mLatest = nullptr;
    }
    CFile & operator = (CFile other) {
        swap(mPos, other.mPos);
        swap(mLatestPos, other.mLatestPos);
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
    uint32_t writeToBuffer(Buffer & target, size_t pos, const Buffer & src) {
        size_t end = pos + src.size();
        for (size_t read = 0; pos < end; pos++, read++) {
            target[pos] = src[read];
        }
        return src.size();
    }
    void truncate(void) {
        checkWrite();
        mCur -> dropFrom(mPos);
    }
    uint32_t fileSize(void) const { return mCur -> size(); };
    void addVersion(void) {
        cout << "Crearing a new version" << endl;
        Vector<Version::Change> changes;
        if (mLatest == mCur) {
            mVersions.add(new Version(mLatestPos, mLatest -> size(), move(changes)));
            mLatestPos = mPos;
            return;
        }
        bool inChange = false;
        Version::Change current;
        size_t maxLen = min(mLatest->size(), mCur -> size());
        for (size_t i = 0; i < maxLen; i++) {
            if ((*mLatest)[i] != (*mCur)[i]) {
                if (!inChange) {
                    current = Version::Change(i);
                    inChange = true;
                }
                current.mBuffer.add((*mLatest)[i]);
            } else {
                if (inChange) {
                    changes.add(move(current));
                    inChange = false;
                }
            }
        }
        if(mLatest->size() > mCur -> size()) {
            if (!inChange) {
                current = Version::Change(mLatest->size());
                inChange = true;
            }
            for (size_t i = mCur -> size(); i < mLatest -> size(); i++)
                current.mBuffer.add((*mLatest)[i]);
        }
        if (inChange) {
            changes.add(move(current));
            inChange = false;
        }

        Version tmp = Version(mLatestPos, mLatest -> size(), changes);
        cout << tmp;
        mVersions.add(new Version(mLatestPos, mLatest -> size(), move(changes)));
        mLatest = mCur;
        mLatestPos = mPos;
    }
    bool undoVersion(void) {
        cout << "Undoing version" << endl;
        if (mVersions.isEmpty()) return false;

        mCur = mLatest;
        mPos = mLatestPos;

        checkWriteLatest();
        Version & version = *(mVersions.pop());
        mLatest->setSize(version.mLen);
        mLatestPos = version.mPos;
        for (const Version::Change & change : version.mChanges) {
            writeToBuffer(*mLatest, change.mIndex, change.mBuffer);
        }
        cout << version;
        return true;
    }
    void printBuffer(ostream & out) const {
        out << "Buf: Len: " << setfill(' ') << setw(2) << mCur -> size() << " Data: ";
        for (size_t i = 0; i < mCur -> size(); i++)
            out << (uint32_t)(*mCur)[i] << ", ";
        out << endl;
        out << "Lts: Len: " << setfill(' ') << setw(2) << mLatest -> size() << " Data: ";
        for (size_t i = 0; i < mLatest -> size(); i++)
            out << (uint32_t)(*mLatest)[i] << ", ";
        out << endl;
    }
    private:
    void checkWrite() {
        if (mCur.hasOne()) return;
        cout << "Copying current buffer" << endl;
        Buffer * copy = new Buffer;
        *copy = *mCur;
        mCur = SPtr(copy);
    }
    void checkWriteLatest() {
        if (mLatest.hasOne()) return;
        cout << "Copying latest buffer" << endl;
        Buffer * copy = new Buffer;
        *copy = *mLatest;
        mLatest = SPtr(copy);
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
    f0.printBuffer(cout);
    f0.addVersion();

    assert( f0.seek(6));
    assert( writeTest(f0, { 100, 101, 102, 103 }, 4));
    f0.printBuffer(cout);
    f0.addVersion();

    assert( f0.seek(5));
    cout << "Copy object" << endl;
    CFile f1(f0);
    f0.printBuffer(cout);
    cout << "Truncate" << endl;
    f0.truncate ();
    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70 }, 20));
    f0.printBuffer(cout); cout << endl;
    cout << "Apply undo" << endl;
    assert( f0.undoVersion ());
    f0.printBuffer(cout);

    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70, 80, 100, 101, 102, 103 }, 20));
    assert( f0.undoVersion ());
    f0.printBuffer(cout); cout << endl;

    assert( f0.seek(0));
    assert( readTest(f0, { 10, 20, 5, 4, 70, 80 }, 20));
    assert( !f0.seek(100));
    assert( writeTest(f1, { 200, 210, 220 }, 3));
    assert( f1.seek(0));
    assert( readTest(f1, { 10, 20, 5, 4, 70, 200, 210, 220, 102, 103 }, 20));
    f0.printBuffer(cout); cout << endl;
    assert( f1.undoVersion ());
    f0.printBuffer(cout); cout << endl;

    assert( f1.undoVersion ());
    f0.printBuffer(cout); cout << endl;

    assert( readTest(f1, { 4, 70, 80 }, 20));
    assert( !f1.undoVersion ());
    f0.printBuffer(cout); cout << endl;

    cout << "All tests padded!" << endl;
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
