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

/**
 * Conterts normal istream reading byte info one reading bits
 */
class ByteInStream {
    private:
        // stream from with are we reading
        istream & mIn;
        //latest byte read
        uint8_t mInByteIndex = 0;
        // index in the byte from the MSB
        uint8_t mByte = 0;

    public:
        /**
         * @param in stream to read bits from
         */
        explicit ByteInStream(istream & in) : mIn(in) {};

        /**
          * Reads the next bit from this stream
          * @return true for 1 and false for 0 bit
          */
        bool readBit() {
            if (!good()) return false;

            // get new byte from stream
            if (mInByteIndex == 0) {
                mByte = mIn.get();
                // cout << "Read byte: " << mByte << " - " << (int) mByte << endl;
            }
            bool val = (mByte & (1u << (7u - mInByteIndex))) > 0;
            // cout << "Getting " << val << ", index: " << inByteIndex << endl;
            mInByteIndex &= ~-(++mInByteIndex == 8);
            return val;
        };

        /**
          * Reads the next byte from this stream
          * @return byte of data
          */
        uint8_t readByte() {
            uint8_t val = 0;
            for (uint8_t i = 0; i < 8; i++) {
                const bool isTrue = readBit();
                val = val << 1;
                if (isTrue) val++;
                // cout << "Connected " << isTrue << " - " << (int)val << endl;
            }
            return val;
        }

        // same as normal streams
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

/**
 * Writes single bits to a stream
 */
class ByteOutStream {
    private:
        // stream to write bits to
        ostream & mOut;
        // wich bit in byte is written right now
        uint8_t mOutByteIndex = 0;
        // current cached byte of bits writen
        uint8_t mByte = 0;

    public:
        explicit ByteOutStream(ostream & out) : mOut(out) {};

        /**
         * writes single bit into a stream
         * @param bit bit to write, true for 1 and false for 0
         */
        void putBit(bool bit) {

            mByte = mByte << 1;
            if (bit) mByte++;
            // flushed whole byte into a stream
            if (mOutByteIndex == 7) {
                mOut.put(mByte);
            }
            //cout << "Putting " << val << " index " << (int)mOutByteIndex << endl;
            mOutByteIndex &= ~-(++mOutByteIndex == 8);
        };

        /**
         * Writes entire byte into the stream
         * @param byte byte to write
         */
        void putByte(uint8_t byte) {
            for (uint8_t i = 0; i < 8; i++) {
                putBit((byte & (1u << (7u - i))) > 0);
            }
        }

        // same as while using normal streams
        bool good() const {
            return mOut.good();
        }
        bool eof() const {
            return mOut.eof();
        }
        bool fail() const {
            return mOut.fail() || mOut.bad();
        }

        /**
         * Flushes the lates byte into a stream, fill the remaing bit with 0
         */
        void close() {
            // fill and flush lastest byte
            while(mOutByteIndex != 0) putBit(false);
        }
};

typedef uint64_t UtfChar;

class UtfParser {
    private:
        ByteInStream & mIn;
    public:
        explicit UtfParser(ByteInStream & in): mIn(in) {}

    private:

        enum class MatchResult {FAIL, ONE, TWO, THREE, FOUR, OTHER};

        struct Pattern {
            const uint8_t pat, inv;
        };

#define staticconst static constexpr const
        staticconst Pattern patOne   = Pattern{0b00000000u, 0b10000000u};
        staticconst Pattern patTwo   = Pattern{0b11000000u, 0b00100000u};
        staticconst Pattern patThree = Pattern{0b11100000u, 0b00010000u};
        staticconst Pattern patFour  = Pattern{0b11110000u, 0b00001000u};
        staticconst Pattern patOther = Pattern{0b10000000u, 0b01000000u};

public:
        bool readUtfChar(UtfChar & target) const {
            uint8_t byte = mIn.readByte();
            MatchResult res = matchByte(byte);
            switch (res) {
                case MatchResult::ONE:
                    target = byte;
                    return true;

                case MatchResult::TWO:
                    return readBytes(byte, 1, target);

                case MatchResult::THREE:
                    return readBytes(byte, 2, target);

                case MatchResult::FOUR:
                    return readBytes(byte, 3, target);
                default:
                    return false;
            }
            return false;
        }

private:
        bool readBytes(const uint8_t alreadyRead, const uint8_t bytesLeft, UtfChar & out) const {
            out = alreadyRead;
            for (uint8_t i = 0; i < bytesLeft; i++) {
                uint8_t byte = mIn.readByte();
                if (!match(byte, patOther))
                    return false;
                out <<= 8;
                out += byte;
            }
            return true;
        }

        MatchResult matchByte(const uint8_t byte) const {
            if (match(byte, patOne))    return MatchResult::ONE;
            if (match(byte, patTwo))    return MatchResult::TWO;
            if (match(byte, patThree))  return MatchResult::THREE;
            if (match(byte, patFour))   return MatchResult::FOUR;
            if (match(byte, patOther))  return MatchResult::OTHER;
            return MatchResult::FAIL;
        }

        bool match(const uint8_t byte, const Pattern pat) const {
            return ((pat.pat & byte) == pat.pat)
                && ((pat.inv & ~byte) == pat.inv);
            /* First nostalgic version
               uint8_t inv = ~byte;
               for (uint8_t i = 0b10000000u; i > 0; i >>= 1) {
               if (((pat.pat & i) && !(byte & i)) || ((pat.inv & i) && !(inv & i)))
               return false;
               }
               */
        }
};

/**
 * Represents one node in compression tree, a letter or a fork
 */
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



/**
 * Holds decompression tree and runs methods on it
 */
class Tree {
    private:
        TNode * mRoot = nullptr;
        ByteInStream & mIn;
        UtfParser mParser;
        bool mFailed = false;

        /**
         * Parses binary tree from input stream
         * @retrun dynamically allocated root item of a tree
         */
        TNode * parseTree();
        /**
         * Frees dynamically allocated tree from memory
         * @param root tree node to free them all
         */
        void free(TNode * node);

    public:
        Tree(ByteInStream & in)
            : mIn(in), mParser({in}) {
                mRoot = parseTree();
            }
        ~Tree() {
            free(mRoot);
            mRoot = nullptr;
        }

        bool failed() const { return mFailed; }
        void printTree(ostream & out) const;
        /**
         * Reads bits from stream and finds corresponding char
         * @param in stream to read from
         * @param letter place to save output to
         */
        void find(UtfChar & letter) const;
    private:
        void find(const TNode * node, UtfChar & letter) const;
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

TNode * Tree::parseTree() {
    if (mFailed) return nullptr;

    bool isLetter = mIn.readBit();
    if (isLetter) {
        //cout << "parsing letter" << endl;
        UtfChar read;
        if (! mParser.readUtfChar(read)) {
            mFailed = true;
            return nullptr;
        }
        return new TNode(read);
    } else {
        //cout << "parsing nodes" << endl;
        return new TNode(parseTree(), parseTree());
    }
}

void Tree::printTree(ostream & out) const {
    cout << "Printing tree" << endl;
    mRoot -> printTree(out);
    cout << "Printing tree done" << endl;
}


inline void Tree::find(UtfChar & letter) const {
    return find(mRoot, letter);
}

void Tree::find(const TNode * node, UtfChar & letter) const {
    if (node -> mIsLetter) {
        letter = node -> mLetter;
    } else {
        if (mIn.readBit() == false)
            return find(node -> mLeft, letter);
        else
            return find(node -> mRight, letter);
    }
}




/**
 * Reads all the chunks in a stream and writes them to a output stream
 * @param tree decompression table to read from
 * @param in stream to read bits, later codes, from
 * @param out stream to write letter to
 * @retrun true if all the operations succeded
 */
bool parseChunks(Tree & tree, ByteInStream & in, ostream & out);
/**
 * Decides how long is the next chunk going to be
 * @param in stream to read bits from
 * @retrun next chunk size
 */
uint16_t readChunkSize(ByteInStream & in);
/**
 * Writes a letter to a stream respecting it's UtfCharacter
 * @param stream to write into
 * @param letter utf char to write
 */
void writeUrfChar(ostream & out, const UtfChar & letter);


const size_t chunkDefSize = 4096;
bool parseChunks(Tree & tree, ByteInStream & in, ostream & out) {
    while(in.good() && out.good()) {
        size_t size = readChunkSize(in);
        //cout << "Chunksize: " << size << endl;

        // go trough chars
        for (size_t i = 0; i < size; i++) {
            UtfChar c;
            tree.find(c);
            writeUrfChar(out, c);
            //cout << "Read char: " << c << " - " << (int)c << endl;
        }
        //latest chunk is always smaller
        if (chunkDefSize != size)
            return true;
    }
    return false;
}

uint16_t readChunkSize(ByteInStream & in) {
    bool isDefault = in.readBit();
    if (isDefault) return chunkDefSize;

    // read 12-bit number
    uint16_t val = 0;
    for (uint8_t i = 0; i < 12; i++) {
        val = val << 1;
        if (in.readBit()) val++;
    }
    return val;
}

void writeUrfChar(ostream & out, const UtfChar & letter) {
    uint64_t mask = 0b11111111u << 24;
    for (; mask > 0; mask >>= 8) {
        uint8_t byte = letter & mask;
        // skip empty bytes except the last one (for '\0' character)
        if (byte != 0 || mask == 0b11111111u)
            out.put(byte);
    }
}


/**
 * Checks if all the operations completed as expected
 * Checks if there are no more bytes in input stream
 * @param streams streams to check
 * @retrun true if everything is good
 */
bool isReadCompletelly(FileStreams & streams);

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
