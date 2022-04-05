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

class CFile {
    public:
        // constructor

        // Size

        // Change
};

class CLink {
    public:
        // constructor

        // Size

        // Change
};

class CDirectory {
    public:
        // constructor

        // Size

        // Change

        // Get

        // operator<<
};

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
