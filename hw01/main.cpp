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
        ByteInStream(istream & in) : mIn(in) {};

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
            val = val << 1;
            if (readBit()) val++;
            //cout << "Connected " << (int)val << endl;
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
        ByteOutStream(ostream & out) : mOut(out) {};

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

typedef size_t NodeCode;
typedef char UtfChar;

class TNode {
    private:
        NodeCode mCode;
        bool mIsLetter;
        UtfChar mLetter;
        TNode * mLeft, * mRight;
    public:
        TNode(NodeCode code, UtfChar c)
            : mCode(code), mIsLetter(true), mLetter(c),
            mLeft(nullptr), mRight(nullptr) {};
        TNode(NodeCode code, TNode * left, TNode * right)
            : mCode(code), mIsLetter(false),  mLetter('\0'),
            mLeft(left), mRight(right) {};

        // bool isLetter()     const { return mIsLetter; }
        // UtfChar getLetter() const { return mLetter;     }
        // TNode * getLeft()   const { return mLeft;     }
        // TNode * getRight()  const { return mRight;    }

        void printTree(ostream & out);
        void free();
        bool find(const NodeCode code, UtfChar & letter) const {
            if (mIsLetter) {
                if (mCode == code) {
                    letter =mLetter;
                    return true;
                } else return false;
            } else {
                //TODO do it smart, faster
                return mLeft -> find(code, letter)
                    || mRight -> find(code, letter);
            }
        }
};

void TNode::printTree(ostream & out) {
    if (mIsLetter) cout << mLetter << " - " << setbase(2) << mCode << endl;
    else {
        mLeft -> printTree(out);
        mRight -> printTree(out);
    }
}
void TNode::free() {
    if (!mIsLetter) {
        mLeft -> free();
        mRight -> free();
        mLeft = nullptr;
        mRight = nullptr;
    } else {
        mLetter = '\0';
    }
    mCode = 0;
    delete this;
}

TNode * parseTree(ByteInStream & in, NodeCode code) {
    bool isLetter = in.readBit();
    if (isLetter) {
        //cout << "parsing letter" << endl;
        return new TNode(code, in.readByte());
    } else {
        //cout << "parsing nodes" << endl;
        code = code << 1;
        return new TNode(code, parseTree(in, code), parseTree(in, code + 1));
    }
}

bool decompressFile ( const char * inFileName, const char * outFileName ) {
    FileStreams streams(inFileName, outFileName);
    if (!streams.good()) { return false; }
    ByteInStream in(streams.getIn());
    ByteOutStream out(streams.getOut());

    TNode * keys = parseTree(in, 0);
    keys -> printTree(cout);
    keys -> free();

    return false;
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

void testTreeParsing() {
    ifstream fIn ("tests/test0.huf", ios::binary);
    ByteInStream in(fIn);
    TNode * tree = parseTree(in, 0);
    UtfChar c;

    assert(tree->find(6, c) && c == 'K');
    assert(tree->find(0, c) && c == 'o');
    assert(tree->find(7, c) && c == 'l');
    assert(tree->find(4, c) && c == 't');
    assert(tree->find(5, c) && c == 'c');

    assert(!tree->find(9, c));

    tree -> free();
}

int main ( void ) {

    testByteInStream();
    testByteOutStream();
    testTreeParsing();

    assert( identicalFiles( "tests/test0.orig", "tests/test0.orig"));
    assert(!identicalFiles( "tests/test0.orig", "tests/test1.orig"));

    return 0;

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
