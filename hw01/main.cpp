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
/**
 * Compresses file using Huffman compression
 * @param inFileName input file
 * @param outFileName file to write in
 * @return if compression succeded
 */
bool compressFile ( const char * inFileName, const char * outFileName );

#endif /* __PROGTEST__ */


class HOut {
    protected:
        HOut() {}
    public:
        virtual void put(uint8_t) {}
        virtual bool good() const { return false; }
        virtual bool eof()  const { return true; }
        virtual bool fail() const { return true; }
        virtual void close() {}
};

class HOutStd : public HOut {
    private:
        ostream & mOut;
    public:
        explicit HOutStd(ostream & out) : mOut(out) {}
        virtual void put(uint8_t byte) { mOut.put(byte); }
        virtual bool good() const { return mOut.good(); }
        virtual bool eof()  const { return mOut.eof(); }
        virtual bool fail() const { return mOut.fail(); }
        virtual void close() {}
};

class HOutFile : public HOut {
    private:
        ofstream & mOut;
    public:
        explicit HOutFile(ofstream & out) : mOut(out) {}
        virtual void put(uint8_t byte) { mOut.put(byte); }
        virtual bool good() const { return mOut.good(); }
        virtual bool eof()  const { return mOut.eof(); }
        virtual bool fail() const { return mOut.fail(); }
        virtual void close() { mOut.close(); }
};

class HIn {
    public:
        virtual uint8_t get() { return 0; }
        virtual bool good() const { return false; };
        virtual bool eof() const { return false; };
        virtual bool fail() const { return true; };
        virtual void close() {}
};

class HInStd : public HIn {
    private:
        istream & mIn;
    public:
        explicit HInStd(istream & in) : mIn(in) {}
        uint8_t get() override { return mIn.get(); }
        bool good() const override { return mIn.good(); }
        bool eof()  const override { return mIn.eof(); }
        bool fail() const override { return mIn.fail(); }
        void close() override {}
};

class HInFile : public HIn {
    private:
        ifstream & mIn;
    public:
        explicit HInFile(ifstream & in) : mIn(in) {}
        uint8_t peek() { return mIn.peek(); }
        uint8_t get() override { return mIn.get(); }
        bool good() const override { return mIn.good(); }
        bool eof()  const override { return mIn.eof(); }
        bool fail() const override { return mIn.fail(); }
        void close() { mIn.close(); }
};


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
            mOut.flush();
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
class BitInStream : public HIn {
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
        explicit BitInStream(istream & in) : mIn(in) {};

        /**
         * Reads the next bit from this stream
         * @return true for 1 and false for 0 bit
         */
         bool readBit();

        /**
         * Reads the next byte from this stream
         * @return byte of data
         */
        uint8_t get();

        // same as normal streams
        bool good() const override { return mInByteIndex != 1 || mIn.good(); }
        bool eof()  const override { return mInByteIndex == 1 && mIn.eof(); }
        bool fail() const override { return !(good() || eof()); }
        void close() {}
};

/**
 * Writes single bits to a stream
 */
class BitOutStream : public HOut {
    private:
        // stream to write bits to
        ostream & mOut;
        // wich bit in byte is written right now
        uint8_t mOutByteIndex = 0;
        // current cached byte of bits writen
        uint8_t mByte = 0;

    public:
        explicit BitOutStream(ostream & out) : mOut(out) {};
        virtual ~BitOutStream() { close(); }

        /**
         * writes single bit into a stream
         * @param bit bit to write, true for 1 and false for 0
         */
        virtual void putBit(bool bit);

        /**
         * Writes entire byte into the stream
         * @param byte byte to write
         */
        virtual void put(uint8_t byte);

        // same as while using normal streams
        virtual bool good() const { return mOut.good(); }
        virtual bool eof() const { return mOut.eof(); }
        virtual bool fail() const { return mOut.fail() || mOut.bad(); }

        /**
         * Flushes the lates byte into a stream, fill the remaing bit with 0
         */
        virtual void close();
};






/**
 * Represents up to 4 bytes of an utf character
 */
typedef uint64_t UtfChar;

class UtfParser {
    private:
        HIn & mIn;
    public:
        explicit UtfParser(HIn & in): mIn(in) {}

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
        static const uint64_t maxUtfValue = 0b11110100100011111011111110111111;

    public:
        bool readUtfChar(UtfChar & target) const;

    private:
        bool gets(const uint8_t alreadyRead, const uint8_t bytesLeft, UtfChar & out) const;

        MatchResult matchByte(const uint8_t byte) const;

        bool match(const uint8_t byte, const Pattern pat) const;
};

/**
 * Represents one node in compression tree, a letter or a fork
 */
class TNode {
    private:
        bool mIsLetter;
        UtfChar mLetter;
        TNode * mLeft, * mRight;
        size_t mOccurance;
    public:
        explicit TNode(UtfChar c, size_t occurance = 0)
            : mIsLetter(true), mLetter(c),
            mLeft(nullptr), mRight(nullptr), mOccurance(occurance) {}

        explicit TNode(TNode * left, TNode * right, size_t occurance = 0)
            : mIsLetter(false),  mLetter('\0'),
            mLeft(left), mRight(right), mOccurance(occurance) {}

        void printTree(HOut & out) const;
        friend class Tree;

        bool operator < (TNode & other) {
            return mOccurance < other.mOccurance;
        }

        class PointerComparator {
            public:
                bool operator() (TNode* n1, TNode* n2) {
                    return n1 -> mOccurance > n2 -> mOccurance;
                }
        };
};



/**
 * Holds decompression tree and runs methods on it
 */
class Tree {
    private:
        TNode * mRoot = nullptr;
        bool mFailed = false;

        /**
         * Parses binary tree from input stream
         * @retrun dynamically allocated root item of a tree
         */
        TNode * parseTree(BitInStream & in, UtfParser & parser);
        TNode * createFromMap(const unordered_map<UtfChar, size_t> & map);
        /**
         * Frees dynamically allocated tree from memory
         * @param root tree node to free them all
         */
        void free(TNode * node);

    public:
        Tree(BitInStream & in) {
            UtfParser mParser(in);
            mRoot = parseTree(in, mParser);
        }
        Tree(const unordered_map<UtfChar, size_t> & map) {
            mRoot = createFromMap(map);
        }
        ~Tree() {
            free(mRoot);
            mRoot = nullptr;
        }

        bool failed() const { return mFailed; }
        void printTree(HOut & out) const;

        void find(BitInStream & in, UtfChar & letter) const;
        void findChar(vector<bool> & position, const UtfChar & letter) const;
        void writeTree(BitOutStream & out) const;
        /**
         * Reads bits from stream and finds corresponding char
         * @param in stream to read from
         * @param letter place to save output to
         */
    private:
        void find(BitInStream & in, const TNode * node, UtfChar & letter) const;
        bool findChar(vector<bool> & position, const TNode * node, const UtfChar & letter) const;
        void writeTree(BitOutStream & out, const TNode * node) const;
};



// --- Final output and chunk parsing ----------------------------------------
/**
 * Reads all the chunks in a stream and writes them to a output stream
 * @param tree decompression table to read from
 * @param in stream to read bits, later codes, from
 * @param out stream to write letter to
 * @retrun true if all the operations succeded
 */
bool parseChunks(Tree & tree, BitInStream & in, ostream & out);
/**
 * Decides how long is the next chunk going to be
 * @param in stream to read bits from
 * @retrun next chunk size
 */
uint16_t readChunkSize(BitInStream & in);
/**
 * Writes a letter to a stream respecting it's UtfCharacter
 * @param stream to write into
 * @param letter utf char to write
 */
void writeUtfChar(HOut & out, const UtfChar & letter);
/**
 * Checks if all the operations completed as expected
 * Checks if there are no more bytes in input stream
 * @param streams streams to check
 * @retrun true if everything is good
 */
bool isReadCompletelly(FileStreams & streams);




// --- Functions definitions --------------------------------------------------

// --- BitInStream -----------------------------------------------------------
bool BitInStream::readBit() {
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
}

uint8_t BitInStream::get() {
    uint8_t val = 0;
    for (uint8_t i = 0; i < 8; i++) {
        const bool isTrue = readBit();
        val = val << 1;
        if (isTrue) val++;
        // cout << "Connected " << isTrue << " - " << (int)val << endl;
    }
    return val;
}


// -- BitOutStream -----------------------------------------------------------
void BitOutStream::putBit(bool bit) {

    mByte = mByte << 1;
    if (bit) mByte++;
    // flushed whole byte into a stream
    if (mOutByteIndex == 7) {
        mOut.put(mByte);
    }
    //cout << "Putting " << val << " index " << (int)mOutByteIndex << endl;
    mOutByteIndex &= ~-(++mOutByteIndex == 8);
}

void BitOutStream::put(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        putBit((byte & (1u << (7u - i))) > 0);
    }
}

void BitOutStream::close() {
    // fill and flush lastest byte
    while(mOutByteIndex != 0) putBit(false);
}





// --- Tree and TNode definitions ---------------------------------------------
void TNode::printTree(HOut & out) const {
    if (mIsLetter) {
        writeUtfChar(out, mLetter);
        //out << " (" << (int) mLetter << ")" << endl;
    }
    else {
        mLeft -> printTree(out);
        mRight -> printTree(out);
    }
}

void Tree::free(TNode * node) {
    if (node != nullptr) {
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
}

TNode * Tree::parseTree(BitInStream & in, UtfParser & parser) {
    if (mFailed || !in.good()) return nullptr;

    bool isLetter = in.readBit();
    if (isLetter) {
        UtfChar read;
        if (! parser.readUtfChar(read)) {
            mFailed = true;
            return nullptr;
        }
        return new TNode(read);
    } else {
        return new TNode(parseTree(in, parser), parseTree(in, parser));
    }
}

void Tree::printTree(HOut & out) const {
    cout << "Printing tree" << endl;
    mRoot -> printTree(out);
    cout << "Printing tree done" << endl;
}

inline void Tree::find(BitInStream & in, UtfChar & letter) const {
    return find(in, mRoot, letter);
}

void Tree::find(BitInStream & in, const TNode * node, UtfChar & letter) const {
    if (node -> mIsLetter) {
        letter = node -> mLetter;
    } else {
        if (in.readBit() == false)
            return find(in, node -> mLeft, letter);
        else
            return find(in, node -> mRight, letter);
    }
}

TNode * Tree::createFromMap(const unordered_map<UtfChar, size_t> & map) {
    priority_queue<TNode*, vector<TNode*>, TNode::PointerComparator> queue;
    for (const auto & [letter, occurance] : map) {
        TNode * node = new TNode(letter, occurance);
        queue.push(node);
        // BitOutStream bCout = BitOutStream(cout);
        // cout << "Inserting ";
        // writeUtfChar(bCout, letter);
        // cout << " (" << occurance << ")" << endl;
    }

    while(true) {
        TNode * first = queue.top();
        queue.pop();
        if (queue.empty()) {
            return first; // top found
        }
        TNode * second = queue.top();
        queue.pop();
        // cout << "poped "
        // << first -> mOccurance << " and "
        // << second -> mOccurance << endl;
        TNode * parent = new TNode(first, second, first -> mOccurance + second -> mOccurance);
        queue.push(parent);
    }
}

void Tree::writeTree(BitOutStream & out) const { return writeTree(out, mRoot); }

void Tree::writeTree(BitOutStream & out, const TNode * node) const {
    if (node -> mIsLetter) {
        out.putBit(true);
        writeUtfChar(out, node -> mLetter);
    } else {
        out.putBit(false);
        writeTree(out, node -> mLeft);
        writeTree(out, node -> mRight);
    }
}

void Tree::findChar(vector<bool> & position, const UtfChar & letter) const {
    position.clear();
    findChar(position, mRoot, letter);
}

bool Tree::findChar(vector<bool> & position, const TNode * node, const UtfChar & letter) const {
    if (node -> mIsLetter) {
        return node -> mLetter == letter;
    } else {
        position.push_back(false);
        if (findChar(position, node -> mLeft, letter)) return true;
        position.back() = true;
        if (findChar(position, node -> mRight, letter)) return true;
        position.pop_back();
        return false;
    }
}



// --- UtfParser definitions --------------------------------------------------
#define MR UtfParser::MatchResult
bool UtfParser::readUtfChar(UtfChar & target) const {
    uint8_t byte = mIn.get();
    MR res = matchByte(byte);
    // cout << "Byte read: " << byte << " (" << (uint64_t) byte << ")" << endl;
    switch (res) {
        case MR::ONE:
            // cout << "Res: 1" << endl;
            target = byte;
            return true;
        case MR::TWO:
            // cout << "Res: 2" << endl;
            return gets(byte, 1, target);
        case MR::THREE:
            // cout << "Res: 3" << endl;
            return gets(byte, 2, target);
        case MR::FOUR:
            // cout << "Res: 4" << endl;
            return gets(byte, 3, target) && target <= UtfParser::maxUtfValue;
        default:
            return false;
    }
    return false;
}

bool UtfParser::gets(const uint8_t alreadyRead, const uint8_t bytesLeft, UtfChar & out) const {
    out = alreadyRead;
    for (uint8_t i = 0; i < bytesLeft; i++) {
        uint8_t byte = mIn.get();
        // cout << "Byte other: " << byte << " (" << (uint64_t) byte << ")" << endl;
        if (!match(byte, patOther))
            return false;
        out <<= 8;
        out += byte;
    }
    return true;
}

MR UtfParser::matchByte(const uint8_t byte) const {
    if (match(byte, patOne))    return MatchResult::ONE;
    if (match(byte, patTwo))    return MatchResult::TWO;
    if (match(byte, patThree))  return MatchResult::THREE;
    if (match(byte, patFour))   return MatchResult::FOUR;
    if (match(byte, patOther))  return MatchResult::OTHER;
    return MatchResult::FAIL;
}

bool UtfParser::match(const uint8_t byte, const Pattern pat) const {
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






// --- Chunk management and output --------------------------------------------
const size_t chunkDefSize = 4096;
bool parseChunks(Tree & tree, BitInStream & in, BitOutStream & out) {
    while(in.good() && out.good()) {
        size_t size = readChunkSize(in);
        //cout << "Chunksize: " << size << endl;

        // go trough chars
        for (size_t i = 0; i < size; i++) {
            UtfChar c;
            tree.find(in, c);
            writeUtfChar(out, c);
            //cout << "Read char: " << c << " - " << (int)c << endl;
        }
        //latest chunk is always smaller
        if (chunkDefSize != size)
            return true;
    }
    return false;
}

uint16_t readChunkSize(BitInStream & in) {
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

void writeUtfChar(HOut & out, const UtfChar & letter) {
    //cout << "Letter: " << letter << endl;
    uint64_t mask = 0b11111111u << 24;
    for (uint8_t i = 0; i < 4; i++, mask >>= 8) {
        uint8_t byte = (letter & mask) >> ((3 - i) * 8);
        //cout << "Mask: " << mask << endl;
        //cout << "Byte: " << (uint16_t) byte << endl;
        // skip empty bytes except the last one (for '\0' character)
        if (byte != 0 || mask == 0b11111111u) {
            //cout << "Putting " << (uint16_t) byte << endl;
            out.put(byte);
        }
    }
}

bool isReadCompletelly(FileStreams & streams) {
    if (!streams.getIn().good()) return false;
    if (!streams.getOut().good()) return false;

    // checks if there aren't more data than required
    streams.getIn().peek();
    return streams.getIn().eof();
}




// --- Asingment --------------------------------------------------------------
bool decompressFile ( const char * inFileName, const char * outFileName ) {
    FileStreams streams(inFileName, outFileName);
    if (!streams.good()) { return false; }
    BitInStream in(streams.getIn());
    BitOutStream out(streams.getOut());

    Tree tree(in);
    //tree.printTree(cout);

    if (tree.failed() || !parseChunks(tree, in, out)) {
        return false;
    }

    if (!isReadCompletelly(streams)) {
        return false;
    }

    return true;
}


bool readToMap(HInFile & in, unordered_map<UtfChar, size_t> & map) {
    UtfParser parser(in);
    UtfChar letter;
    while (in.good()) {
        in.peek();
        if (in.eof()) return true;
        if (!parser.readUtfChar(letter) || !in.good()) return false;

        if (map.find(letter) == map.end())
            map[letter] = 0;
        map[letter]++;
    }
    return false;
}

uint16_t readChunk(HIn & in, UtfParser & parser, UtfChar * chunk) {

    UtfChar letter;
    uint16_t i = 0;
    for (; i < chunkDefSize && in.good(); i++) {
        parser.readUtfChar(letter); // file already read once, bytes should be valid
        chunk[i] = letter;
        // cout << "Read " << letter << endl;
    }
    return in.good() ? i : i - 1;
}

void write12bitNumber(BitOutStream & out, const uint16_t num) {
    for (uint16_t mask = 1u << 11; mask > 0; mask >>= 1) {
        out.putBit((num & mask) > 0);
    }
}

void writeCharacters(BitOutStream & out, const Tree & tree,
        const UtfChar * chunk, const uint16_t size, vector<bool> & position) {
    for (uint16_t i = 0; i < size; i++) {
        tree.findChar(position, chunk[i]);
        // cout << "Writing char " << (char) chunk[i] << " - ";
        for (bool bit : position) {
            out.putBit(bit);
            // cout << noboolalpha << bit;
        }
        // cout << endl;
    }
}

bool writeToFile(HIn & in, BitOutStream & out, const Tree & tree) {
    UtfChar chunk[chunkDefSize];
    UtfParser parser(in);
    vector<bool> position;
    uint16_t chunkSize;
    do {
        if (!in.good()) return false;
        chunkSize = readChunk(in, parser, chunk);
        // cout << "Writing chunk " << chunkSize << " chars long" << endl;
        if (chunkSize == chunkDefSize) {
            out.putBit(true);
        } else {
            out.putBit(false);
            write12bitNumber(out, chunkSize);
        }
        writeCharacters(out, tree, chunk, chunkSize, position);
    } while(chunkSize == chunkDefSize);
    return true;
}

bool isSuccessfullyCompressed(FileStreams & streams) {
    return streams.getIn().eof() && streams.getOut().good();
}

bool compressFile ( const char * inFileName, const char * outFileName ) {
    FileStreams streams(inFileName, outFileName);
    streams.getIn().peek(); // to check empty file, eof() will be true
    if (!streams.good()) { return false; }

    unordered_map<UtfChar, size_t> map; // of letter occurance
    HInFile fIn(streams.getIn());
    if (!readToMap(fIn, map)) return false;

    streams.getIn().seekg(0);
    //BitInStream in(streams.getIn());
    BitOutStream out(streams.getOut());

    Tree tree(map);
    // tree.printTree(cout);
    tree.writeTree(out);

    if (!writeToFile(fIn, out, tree)) return false;
    if (!isSuccessfullyCompressed(streams)) return false;

    return true;
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

bool sameLengthFiles( const char * fileName1, const char * fileName2 ) {
    ifstream in1(fileName1), in2(fileName2);
    while (true) {
        in1.get();
        in2.get();
        if (in1.good() && in2.good()) {
        } else if (in1.eof() && in2.eof()) {
            return true;
        } else return false;
    }
}

void testBitInStream() {

    istringstream sStream("abc");
    BitInStream bStream(sStream);

    assert (bStream.get() == 'a');
    assert (bStream.get() == 'b');
    assert (bStream.get() == 'c');
    assert ( bStream.good());
    assert (!bStream.eof());
    assert (!bStream.fail());
    bStream.readBit();
    assert (!bStream.good());
    assert ( bStream.eof());
    assert (!bStream.fail());
}

void testBitOutStream() {
    stringstream sOut;
    BitOutStream bOut(sOut);
    bOut.put('a');
    bOut.put('b');
    bOut.put('c');
    assert( bOut.good());
    assert(!bOut.fail());
    assert(sOut.str() == "abc");
}

int main ( void ) {

    testBitInStream();
    testBitOutStream();


    assert( identicalFiles( "tests/test0.orig", "tests/test0.orig"));
    assert(!identicalFiles( "tests/test0.orig", "tests/test1.orig"));
    assert( sameLengthFiles( "tests/test0.orig", "tests/test0.orig"));
    assert(!sameLengthFiles( "tests/test0.orig", "tests/test1.orig"));


    // Decompression tests
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



    // Compression tests
    assert(!compressFile( "/dev/null", "tempcomp" ));

    assert( compressFile(   "tests/test0.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/test0.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/test0.huf", "tempcomp" ));

    assert( compressFile(   "tests/test1.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/test1.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/test1.huf", "tempcomp" ));

    assert( compressFile(   "tests/test2.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/test2.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/test2.huf", "tempcomp" ));

    assert( compressFile(   "tests/test3.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/test3.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/test3.huf", "tempcomp" ));

    assert( compressFile(   "tests/test4.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/test4.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/test4.huf", "tempcomp" ));


    assert( compressFile(   "tests/extra0.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra0.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra0.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra1.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra1.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra1.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra2.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra2.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra2.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra3.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra3.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra3.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra4.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra4.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra4.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra5.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra5.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra5.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra6.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra6.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra6.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra7.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra7.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra7.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra8.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra8.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra8.huf", "tempcomp" ));

    assert( compressFile(   "tests/extra9.orig", "tempcomp" ));
    assert( decompressFile( "tempcomp",   "tempfile" ));
    assert( identicalFiles( "tests/extra9.orig", "tempfile" ));
    assert( sameLengthFiles( "tests/extra9.huf", "tempcomp" ));


    return 0;
}
#endif /* __PROGTEST__ */
