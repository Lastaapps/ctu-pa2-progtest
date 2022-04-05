#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

struct File {
    public:
        virtual ~File() {}
        virtual size_t Size() const = 0;
        virtual string ToName(const string & name) const = 0;
        virtual File * Clone() const = 0;
};

class CFile : public File {
    private:
        string mHast;
        size_t mSize;
    public:
        CFile(const string & hash, const size_t size)
            : mHast(hash), mSize(size) {}
        ~CFile() = default;
        CFile & Change(const string & hash, const size_t size) {
            mHast = hash;
            mSize = size;
            return * this;
        }
        size_t Size() const override { return mSize; }
        string ToName(const string & name) const override {
            return name + " " + mHast;
        }
        virtual File * Clone() const { return new CFile(mHast, mSize); }
};

class CLink : public File {
    private:
        string mPath;
    public:
        CLink(const string & path) : mPath(path) {}
        ~CLink() = default;
        CLink & Change(const string & path) {
            mPath = path;
            return *this;
        }
        size_t Size() const override { return mPath.size() + 1; }
        string ToName(const string & name) const override {
            return name + " -> " + mPath;
        }
        virtual File * Clone() const { return new CLink(mPath); }
};

class CDirectory : public File {
    private:
        map<string, unique_ptr<File>> mMap;
    public:
        CDirectory() {}
        CDirectory(const CDirectory & dir) {
            for (const auto & [name, file] : dir.mMap)
                mMap.emplace(make_pair(name, file -> Clone()));
        }
        ~CDirectory() {};
        CDirectory & operator=(CDirectory source) {
            swap(mMap, source.mMap);
            return *this;
        }
        CDirectory & Change(const string & filename, const File & file) {
            auto iter = mMap.find(filename);
            File * newFile = file.Clone();
            if (iter != mMap.end()) {
                mMap.erase(iter);
            }
            mMap.emplace(make_pair(filename, newFile));
            return *this;
        }
        CDirectory & Change(const string & filename, const File * file) {
            // delete
            if (file == nullptr) {
                auto iter = mMap.find(filename);
                if (iter != mMap.end()) {
                    mMap.erase(iter);
                }
            } else {
                auto iter = mMap.find(filename);
                if (iter != mMap.end())
                    mMap.erase(iter);
                mMap.emplace(make_pair(filename, file -> Clone()));
            }
            return *this;
        }
        File & Get(const string & filename) {
            const auto itr = mMap.find(filename);
            if (itr == mMap.end()) throw out_of_range("Filename not found");
            return *((*itr).second);
        }
        const File & Get(const string & filename) const {
            const auto itr = mMap.find(filename);
            if (itr == mMap.end()) throw out_of_range("Filename not found");
            return *((*itr).second);
        }
        size_t Size() const override {
            size_t total = 0;
            for (const auto & [name, file] : mMap) {
                total += name.size();
                total += file -> Size();
            }
            return total;
        }
        string ToName(const string & name) const override {
            return name + "/";
        }
        virtual File * Clone() const {
            return new CDirectory(*this);
        }
        friend ostream & operator << (ostream & out, const CDirectory & dir);
};

ostream & operator << (ostream & out, const CDirectory & dir) {
    vector<pair<size_t, string>> data;
    for (const auto & [name, file] : dir.mMap)
        data.push_back({file -> Size(), file -> ToName(name)});
    auto sorter = [](const pair<size_t, string> & p1, const pair<size_t, string> & p2) {
        return p1.second < p2.second;
    };
    sort(data.begin(), data.end(), sorter);
    for (const pair<size_t, string> & item : data)
        out << item.first << '\t' << item.second << '\n';
    out.flush();
    return out;
}

#ifndef __PROGTEST__
int main () {
    CDirectory root;
    stringstream sout;

    root.Change("file.txt", CFile("jhwadkhawkdhajwdhawhdaw=", 1623))
        .Change("file.ln", CLink("").Change("file.txt"))
        .Change("folder", CDirectory()
                .Change("fileA.txt", CFile("", 0).Change("skjdajdakljdljadkjwaljdlaw=", 1713))
                .Change("fileB.txt", CFile("kadwjkwajdwhoiwhduqwdqwuhd=", 71313))
                .Change("fileC.txt", CFile("aihdqhdqudqdiuwqhdquwdqhdi=", 8193))
               );

    sout.str( "" );
    sout << root;
    assert( sout.str() ==
            "9\tfile.ln -> file.txt\n"
            "1623\tfile.txt jhwadkhawkdhajwdhawhdaw=\n"
            "81246\tfolder/\n" );
    assert( root.Size() == 82899 );

    root.Change("file.txt", CFile("Sel pes do lesa", 42069));
    sout.str( "" );
    sout << root;
    assert( sout.str() ==
            "9\tfile.ln -> file.txt\n"
            "42069\tfile.txt Sel pes do lesa\n"
            "81246\tfolder/\n" );

    root.Change("file.txt", nullptr);
    sout.str( "" );
    sout << root;
    assert( sout.str() ==
            "9\tfile.ln -> file.txt\n"
            "81246\tfolder/\n" );

    string filename = "folder";
    const CDirectory & inner = dynamic_cast<const CDirectory &>( root.Get( filename ) );

    sout.str( "" );
    sout << inner;
    assert( sout.str() ==
            "1713\tfileA.txt skjdajdakljdljadkjwaljdlaw=\n"
            "71313\tfileB.txt kadwjkwajdwhoiwhduqwdqwuhd=\n"
            "8193\tfileC.txt aihdqhdqudqdiuwqhdquwdqhdi=\n" );
    assert( inner.Size() == 81246 );

    return 0;
}
#endif /* __PROGTEST__ */
