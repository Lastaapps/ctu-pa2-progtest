#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
#include <memory>
#include <functional>
#include <stdexcept>
using namespace std;


/**
 * Decompresses a file using Huffman coding and save the output to another one
 * @param inFileName the fileName of the input compressed file
 * @param outFileName the fileName of the output decompressed file
 * @return if decopression succeded
 */
bool decompressFile ( const char * inFileName, const char * outFileName );

#endif /* __PROGTEST__ */

#define PRINT if(true)


/**
 * Stores in and out file streams
 */
class FileStreams {
    private:
        // file streams
        ifstream mIn;
        ofstream mOut;
    public:
        explicit FileStreams( const char * inFileName, const char * outFileName )
            : mIn(ifstream(inFileName, ios::binary)),
            mOut(ofstream(outFileName, ios::binary)) {}
        // closes sreams when they are no longer needed
        ~FileStreams() {
            mIn.close();
            mOut.close();
        }

        /**
         * Checks streams
         * @return if streams are opened and good
         */
        bool good() const {
            return mIn.is_open() && mOut.is_open() && mIn.good() && mOut.good();
        }

        // getters
        /** @return In file stream */
        ifstream & getIn() { return mIn; }
        /** @return Out file stream */
        ofstream & getOut() { return mOut; }
};

class ByteInStream {
    private:
        istream & mIn;
        uint8_t mInByteIndex = 0, mByte = 0;

    public:
        explicit ByteInStream(istream & in) : mIn(in) {};

    bool readBit() {
        if (!good()) return false;

        if (mInByteIndex == 0) {
            mByte = mIn.get();
            //cout << "Read byte: " << mByte << " - " << (int) mByte << endl;
        }
        bool val = (mByte & (1u << (7u - mInByteIndex))) > 0;
        //cout << "Getting " << val << ", index: " << inByteIndex << endl;
        mInByteIndex &= ~-(++mInByteIndex == 8);
        return val;
    };

    uint8_t readByte() {
        uint8_t val = 0;
        for (uint8_t i = 0; i < 8; i++) {
            const bool isTrue = readBit();
            val = val << 1;
            if (isTrue) val++;
            //cout << "Connected " << isTrue << " - " << (int)val << endl;
        }
        return val;
    }

    bool good() const {
        return mInByteIndex != 1 || mIn.good();
    }
    bool eof() const {
        return mInByteIndex == 1 && mIn.eof();
    }
    bool fail() const {
        return !(good() || eof());
    }
};

class ByteOutStream {
    private:
        ostream & mOut;
        uint8_t mOutByteIndex = 0, mByte = 0;

    public:
        explicit ByteOutStream(ostream & out) : mOut(out) {};

    void putBit(bool val) {

        mByte = mByte << 1;
        if (val) mByte++;
        if (mOutByteIndex == 7) {
            mOut.put(mByte);
        }
        //cout << "Putting " << val << " index " << (int)mOutByteIndex << endl;
        mOutByteIndex &= ~-(++mOutByteIndex == 8);
    };

    void putByte(uint8_t byte) {
        for (uint8_t i = 0; i < 8; i++) {
            putBit((byte & (1u << (7u - i))) > 0);
        }
    }

    bool good() const {
        return mOut.good();
    }
    bool fail() const {
        return !good();
    }
    void close() {
        // fill and flush lastest byte
        while(mOutByteIndex != 0) putBit(false);
    }
};

typedef char UtfChar;

class TNode {
    private:
        bool mIsLetter;
        UtfChar mLetter;
        TNode * mLeft, * mRight;
    public:
        explicit TNode(UtfChar c)
            : mIsLetter(true), mLetter(c),
            mLeft(nullptr), mRight(nullptr) {};
        explicit TNode(TNode * left, TNode * right)
            : mIsLetter(false),  mLetter('\0'),
            mLeft(left), mRight(right) {};

        void printTree(ostream & out) const;
        friend class Tree;
};

void TNode::printTree(ostream & out) const {
    if (mIsLetter) cout << mLetter << " (" << (int) mLetter << ")" << endl;
    else {
        mLeft -> printTree(out);
        mRight -> printTree(out);
    }
}




class Tree {
    private:
        TNode * mRoot = nullptr;

        TNode * parseTree(ByteInStream & in);
        void free(TNode * node);
    public:
        Tree(ByteInStream & in) {
            mRoot = parseTree(in);
        }
        ~Tree() {
            free(mRoot);
            mRoot = nullptr;
        }
        void printTree(ostream & out) const;
        void find(ByteInStream & in, UtfChar & letter) const;
    private:
        void find(const TNode * node, ByteInStream & in, UtfChar & letter) const;
};

void Tree::free(TNode * node) {
    if (!(node -> mIsLetter)) {
        free(node -> mLeft);
        free(node -> mRight);
        node -> mLeft = nullptr;
        node -> mRight = nullptr;
    } else {
        node -> mLetter = '\0';
    }
    delete node;
}

TNode * Tree::parseTree(ByteInStream & in) {
    bool isLetter = in.readBit();
    if (isLetter) {
        //cout << "parsing letter" << endl;
        return new TNode(in.readByte());
    } else {
        //cout << "parsing nodes" << endl;
        return new TNode(parseTree(in), parseTree(in));
    }
}

void Tree::printTree(ostream & out) const {
    cout << "Printing tree" << endl;
    mRoot -> printTree(out);
    cout << "Printing tree done" << endl;
}


inline void Tree::find(ByteInStream & in, UtfChar & letter) const {
    return find(mRoot, in, letter);
}

void Tree::find(const TNode * node, ByteInStream & in, UtfChar & letter) const {
    if (node -> mIsLetter) {
        letter = node -> mLetter;
    } else {
        if (in.readBit() == false)
            return find(node -> mLeft, in, letter);
        else
            return find(node -> mRight, in, letter);
    }
}

bool parseChunks(Tree & tree, ByteInStream & in, ostream & out);
uint16_t readChunkSize(ByteInStream & in);
bool isReadCompletelly(FileStreams & streams);


const size_t chunkDefSize = 4096;
bool parseChunks(Tree & tree, ByteInStream & in, ostream & out) {
    while(in.good() && out.good()) {
        size_t size = readChunkSize(in);
        //cout << "Chunksize: " << size << endl;

        for (size_t i = 0; i < size; i++) {
            UtfChar c;
            tree.find(in, c);
            out.put(c);
            //cout << "Read char: " << c << " - " << (int)c << endl;
        }
        if (chunkDefSize != size)
            return true;
    }
    return false;
}

uint16_t readChunkSize(ByteInStream & in) {
    bool isDefault = in.readBit();
    if (isDefault) return chunkDefSize;

    uint16_t val = 0;
    for (uint8_t i = 0; i < 12; i++) {
        val = val << 1;
        if (in.readBit()) val++;
    }
    return val;
}

bool isReadCompletelly(FileStreams & streams) {
    if (!streams.getIn().good()) return false;
    if (!streams.getOut().good()) return false;

    // checks if there aren't more data than required
    streams.getIn().peek();
    return streams.getIn().eof();
}

bool decompressFile ( const char * inFileName, const char * outFileName ) {
    FileStreams streams(inFileName, outFileName);
    if (!streams.good()) { return false; }
    ByteInStream in(streams.getIn());
    //ByteOutStream out(streams.getOut());

    Tree tree(in);
    tree.printTree(cout);

    if (!parseChunks(tree, in, streams.getOut())) {
        return false;
    }

    if (!isReadCompletelly(streams)) {
        return false;
    }

    return true;
}




bool compressFile ( const char * inFileName, const char * outFileName ) {
    return false;
}

#ifndef __PROGTEST__
bool identicalFiles ( const char * fileName1, const char * fileName2 ) {
    ifstream in1(fileName1), in2(fileName2);
    while (true) {
        char c1 = in1.get();
        char c2 = in2.get();
        if (in1.good() && in2.good()) {
            if (c1 != c2) return false;
        } else if (in1.eof() && in2.eof()) {
            return true;
        } else
            return false;
    }
}

void testByteInStream() {

    istringstream sStream("abc");
    ByteInStream bStream(sStream);

    assert (bStream.readByte() == 'a');
    assert (bStream.readByte() == 'b');
    assert (bStream.readByte() == 'c');
    assert ( bStream.good());
    assert (!bStream.eof());
    assert (!bStream.fail());
    bStream.readBit();
    assert (!bStream.good());
    assert ( bStream.eof());
    assert (!bStream.fail());
}

void testByteOutStream() {
    stringstream sOut;
    ByteOutStream bOut(sOut);
    bOut.putByte('a');
    bOut.putByte('b');
    bOut.putByte('c');
    assert( bOut.good());
    assert(!bOut.fail());
    assert(sOut.str() == "abc");
}

int main ( void ) {

    testByteInStream();
    testByteOutStream();

    assert( identicalFiles( "tests/test0.orig", "tests/test0.orig"));
    assert(!identicalFiles( "tests/test0.orig", "tests/test1.orig"));

    assert( decompressFile( "tests/test0.huf",  "tempfile" ));
    assert( identicalFiles( "tests/test0.orig", "tempfile" ));

    assert( decompressFile( "tests/test1.huf",  "tempfile" ));
    assert( identicalFiles( "tests/test1.orig", "tempfile" ));

    assert( decompressFile( "tests/test2.huf",  "tempfile" ));
    assert( identicalFiles( "tests/test2.orig", "tempfile" ));

    assert( decompressFile( "tests/test3.huf",  "tempfile" ));
    assert( identicalFiles( "tests/test3.orig", "tempfile" ));

    assert( decompressFile( "tests/test4.huf",  "tempfile" ));
    assert( identicalFiles( "tests/test4.orig", "tempfile" ));

    assert(!decompressFile( "tests/test5.huf",  "tempfile" ));

    return 0;

    assert( decompressFile( "tests/extra0.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra0.orig", "tempfile" ));

    assert( decompressFile( "tests/extra1.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra1.orig", "tempfile" ));

    assert( decompressFile( "tests/extra2.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra2.orig", "tempfile" ));

    assert( decompressFile( "tests/extra3.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra3.orig", "tempfile" ));

    assert( decompressFile( "tests/extra4.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra4.orig", "tempfile" ));

    assert( decompressFile( "tests/extra5.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra5.orig", "tempfile" ));

    assert( decompressFile( "tests/extra6.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra6.orig", "tempfile" ));

    assert( decompressFile( "tests/extra7.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra7.orig", "tempfile" ));

    assert( decompressFile( "tests/extra8.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra8.orig", "tempfile" ));

    assert( decompressFile( "tests/extra9.huf",  "tempfile" ));
    assert( identicalFiles( "tests/extra9.orig", "tempfile" ));

    return 0;
}
#endif /* __PROGTEST__ */
