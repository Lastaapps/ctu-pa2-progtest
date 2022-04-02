#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
using namespace std;
#endif /* __PROGTEST__ */

/**
 * Represents a shared pointer
 * shares resources among many destinations
 * uses counting references internally
 * no arrays suppored as type T!
 */
template<typename T>
class SPtr {
    private:
        /**
         * Used for reference counting
         * shared among more shared pointers
         */
        class Counter {
            public:
                size_t counter = 0;
        };
        // data stored
        T * mPtr;
        // common counter
        Counter * mCnt;
    public:
        /** Constructs an empty shared pointer*/
        SPtr() : mPtr(nullptr), mCnt(new Counter) { mCnt -> counter ++; }
        /** Constructs a normal pointer holding shared pointer
         * pointer gets delete after no shared points at it */
        SPtr(T * ptr) : mPtr(ptr), mCnt(new Counter) { mCnt -> counter ++; }
        /** Copies internal data, increases reefrence counter */
        SPtr(const SPtr & src) {
            mPtr = src.mPtr;
            mCnt = src.mCnt;
            mCnt -> counter ++;
        }
        /** Moves internal data, increases reefrence counter */
        SPtr(SPtr && src) {
            if (&src == this) return;
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
        /**
         * Deletes holded data in a shared pointer if there
         * is no other reference to it
         */
        ~SPtr() {
            if (mCnt != nullptr) {
                mCnt -> counter --;
                if (mCnt -> counter == 0) {
                    delete mCnt;
                    delete mPtr;
                }
            } else delete mPtr;
            mCnt = nullptr;
            mPtr = nullptr;
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
        /**
         * @return if this shared counter is shared among 1 owner only
         */
        bool hasOne() const { return mCnt -> counter == 1; }
};

/**
 * Replacement for basic std::vector<T>,
 * but with normal API (only necesarry API parts implemented)
 */
template<typename T>
class Vector {
    private:
        T * mArray = nullptr;
        size_t mLen = 0, mCap = 0;
    public:
        /* Creates an empry vector */
        Vector() {}
        /* Copies data from another vector using = operator */
        Vector(const Vector & other) {
            mArray = new T [other.mLen];
            mLen = other.mLen;
            mCap = other.mLen;
            for (size_t i = 0; i < other.mLen; i++)
                mArray[i] = other.mArray[i];
        }
        /* Moves data from another vector */
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
        /** Clear all the internal data */
        ~Vector() {
            delete [] mArray;
            mArray = nullptr;
            mLen = 0;
            mCap = 0;
        }
        /** Adds an item to a vector using copy = operator
         * @param item item to copy and add
         * @return this vector
         */
        Vector & add(const T & item) {
            expandToLen(mLen);
            mArray[mLen++] = item;
            return *this;
        }
        /** Moves and adds an item to a vector using move = operator
         * @param item item to move
         * @return this vector
         */
        Vector & add(T && item) {
            expandToLen(mLen);
            mArray[mLen++] = item;
            return *this;
        }
        /**
         * Invalides/deletes data from index given
         * @param index index to delete items from
         * @return this vector
         */
        Vector & dropFrom(size_t index) {
            mLen = index;
            return *this;
        }

        /** Gets an item on the index given
         * index can be out of bounds
         * @param index index of the item
         * @return witteable reference to a vector item requested
         */
        T & get(const size_t index) {
            workAferEnd(index);
            return mArray[index];
        }
        /** Gets an item on the index given
         * index can NOT be out of bounds
         * @param index index of the item
         * @return const reference to a vector item requested
         */
        const T & get(const size_t index) const {
            assert(index < mLen);
            return mArray[index];
        }
        /** Removes the last item in a vector
         * @return the last item
         */
        T & pop() {
            T & toReturn = get(lastIndex());
            mLen--;
            return toReturn;
        }
        /** Gets an item on the index given
         * index can be out of bounds
         * @param index index of the item
         * @return witteable reference to a vector item requested
         */
        inline T & operator[](const size_t index) { return get(index); }
        /** Gets an item on the index given
         * index can NOT be out of bounds
         * @param index index of the item
         * @return const reference to a vector item requested
         */
        inline const T & operator[](const size_t index) const { return get(index); }
        /** Trims non needed allocated capacity */
        void trimCapacity() {
            if (mLen == 0) {
                clear();
                return;
            }
            if (mLen == mCap) return;
            mCap = mLen;
            applyNewCapacity();
        }
        /** Sets vector size - prepares resources or invalidates old items
         * @param newLen the new size of the vector
         */
        void setSize(const size_t newLen) {
            expandToLen(newLen);
            mLen = newLen;
        }
        /** Deletes all the items in a vector, invalides the data */
        void clear() {
            delete [] mArray;
            mArray = nullptr;
            mLen = 0;
            mCap = 0;
        }
        /** @return index of the last item or 0 for empty list */
        size_t lastIndex() const { return isEmpty() ? 0 : mLen - 1; }
        /** @return the size of a vector */
        inline size_t size() const { return mLen; }
        /** @return if a vector is empty */
        inline bool isEmpty() const { return mLen == 0; }
    private:
        /** Prepares for more items */
        void expandToLen(const size_t len) {
            if (len >= mCap) {
                do {
                    mCap = mCap * 4 / 3 + 8;
                } while(len >= mCap);
                applyNewCapacity();
            }
        }
        /** Moves items to a larger array */
        void applyNewCapacity() {
            T * newArray = new T [mCap];
            for (size_t i = 0; i < mLen; i++)
                newArray[i] = move(mArray[i]);
            delete [] mArray;
            mArray = newArray;
        }
        /** Gets ready for item manipulation out of bounds
         * @param index index to work at, to get ready
         */
        void workAferEnd(const size_t index) {
            if (index >= mLen) {
                expandToLen(index + 1);
                mLen = index + 1;
            }
        }
    public:
        /**
         * Enables iteration through a vector */
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
        /** Returs an iterator to the first item of a vector */
        Iterator begin() { return Iterator(mArray); }
        /** Returs an iterator to the place after the last item of a vector */
        Iterator end() { return Iterator(mArray + mLen); }
};

/**
 * Buffer is a vector of uint8_t
 */
typedef Vector<uint8_t> Buffer;

/** Represents a version change*/
class Version {
    private:
        /** Represents one diff section between two files */
        class Change {
            private:
                // index, where the change hapenned in a file
                size_t mIndex;
                // data to paset on the index
                Buffer mBuffer;
            public:
                Change() : mIndex(0) {}
                Change(size_t index) : mIndex(index) {}
                friend class CFile;
                friend ostream & operator<<(ostream & out, Version & data);
        };
        // where was a curson while making a version
        size_t mPos;
        // how long were the data at the time of making a version
        size_t mLen;
        // list of individual changes/patches
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
    private:
    // write and read cursors
    size_t mPos = 0, mLatestPos = 0;
    // all the verson applied on the file
    Vector<SPtr<Version>> mVersions;
    // buffers of file content data
    // mCur - the current state
    // mLatest - state when addVerson was called
    SPtr<Buffer> mCur = new Buffer, mLatest = new Buffer;

    public:
    /** Creats an empty file buffer */
    CFile(void) {}
    /** Copies all the internal data as shared pointers when posible */
    CFile(const CFile & other)
        : mPos(other.mPos), mLatestPos(other.mLatestPos),
        mVersions(other.mVersions),
        mCur(other.mCur), mLatest(other.mLatest) {}
    /** Moves all the internal data from another object */
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

    /** Moves write and read cursor to a position in the file
     * @param offset position from the start of a file
     * @return true if offset matches current buffer size, false othervise
     */
    bool seek(uint32_t offset) {
        if (offset > mCur -> size()) return false;
        mPos = offset;
        return true;
    }
    /** Reads data from this file
     * @param dst array to store data to
     * @param bytes how many bytes can be read at most
     * @return how many bytes was actually read
     */
    uint32_t read(uint8_t * dst, uint32_t bytes) {
        if (mCur -> isEmpty()) return 0;
        size_t maxBound = min(mCur -> size(), mPos + bytes);
        size_t read = 0;
        for (; mPos < maxBound; mPos++, read++)
            dst[read] = (*mCur)[mPos];
        return read;
    }
    /** Write data to this file
     * @param src where to read data from
     * @param bytes how many bytes should be read
     * @return how many byte has been written
     */
    uint32_t write(const uint8_t * src, uint32_t bytes) {
        checkWrite();
        size_t end = mPos + bytes;
        for (size_t read = 0; mPos < end; mPos++, read++)
            (*mCur)[mPos] = src[read];
        return bytes;
    }
    /**
     * Cuts data from the current cursor position
     */
    void truncate(void) {
        checkWrite();
        mCur -> dropFrom(mPos);
    }
    /** Returns how many bytes are in a file
     * @return file size
     */
    uint32_t fileSize(void) const { return mCur -> size(); };
    /** Stores the current file state to a memory */
    void addVersion(void) {
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
                    current.mBuffer.trimCapacity();
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
            current.mBuffer.trimCapacity();
            changes.add(move(current));
            inChange = false;
        }
        mVersions.add(new Version(mLatestPos, mLatest -> size(), move(changes)));
        mLatest = mCur;
        mLatestPos = mPos;
    }
    /** Restores the lates version of a file stored */
    bool undoVersion(void) {
        if (mVersions.isEmpty()) return false;

        mCur = mLatest;
        mPos = mLatestPos;
        mCur -> trimCapacity();

        checkWriteLatest();
        Version & version = *(mVersions.pop());
        mLatest->setSize(version.mLen);
        mLatestPos = version.mPos;
        for (const Version::Change & change : version.mChanges) {
            writeToBuffer(*mLatest, change.mIndex, change.mBuffer);
        }
        mLatest -> trimCapacity();
        mVersions.trimCapacity();
        return true;
    }
    /** Prints both internal buffers to the stream given
     * uses 2 lines, is ended by a new line and flush
     * @param out stream to print data to
     */
    void printBuffers(ostream & out) const {
        out << "Buf: Len: " << mCur -> size() << " Data: ";
        for (size_t i = 0; i < mCur -> size(); i++)
            out << (uint32_t)(*mCur)[i] << ", ";
        out << endl;
        out << "Lts: Len: " << mLatest -> size() << " Data: ";
        for (size_t i = 0; i < mLatest -> size(); i++)
            out << (uint32_t)(*mLatest)[i] << ", ";
        out << endl;
    }
    private:
    /** Writes data to a buffer at position
     * @param target where to write data to
     * @param pos from where shloud be data written to
     * @param where data should be read from
     * @return how many bytes was written
     */
    uint32_t writeToBuffer(Buffer & target, size_t pos, const Buffer & src) {
        size_t end = pos + src.size();
        for (size_t read = 0; pos < end; pos++, read++)
            target[pos] = src[read];
        return src.size();
    }
    /** Resolves if current buffer can be written
     * to without editing another file copied from thisone.
     * Checks shared pointer, if it has only 1 reference.
     * Otherwise makes a deep copy */
    void checkWrite() {
        if (mCur.hasOne()) return;
        Buffer * copy = new Buffer;
        *copy = *mCur;
        mCur = SPtr(copy);
    }
    /** Resolves if latest buffer can be written
     * to without editing another file copied from thisone.
     * Checks shared pointer, if it has only 1 reference.
     * Otherwise makes a deep copy */
    void checkWriteLatest() {
        if (mLatest.hasOne()) return;
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

    cout << "All tests padded!" << endl;
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
