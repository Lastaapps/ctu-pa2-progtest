#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <variant>
#include <any>
using namespace std;
#endif /* __PROGTEST__ */

typedef set<size_t> NumSet;

template <typename T, typename C = less<typename T::value_type>>
class CIndex {
    private:
        T data;
        C cmp;
        vector<size_t> hashes;
    public:
        CIndex(const T & d) : data(d), cmp(C()) {
            init();
        }
        CIndex(const T & d, const C & c) : data(d), cmp(c) {
            init();
        }
        inline NumSet search(const T & phrase) const {
            if constexpr(is_same_v<T, string> && is_same_v<C, less<char>>)
                return searchString(phrase);
            else return searchGeneral(phrase);
        }
    private:
        static constexpr size_t MODULO = 131;
        static constexpr size_t MULTIPLY = 128;
        void init() {
            if constexpr(is_same_v<T, string> && is_same_v<C, less<char>>)
                createHashes(data);
        }
        inline void createHashes(const string & str) {
            size_t hash = 0;
            hashes.emplace_back(0);
            // cout << "Hash: " << "-  \t0" << endl;
            for (const char c : str) {
                hash *= MULTIPLY;
                hash += c - 'a';
                hash %= MODULO;
                hashes.emplace_back(hash);
                // cout << "Hash: " << c << " \t" << hash << endl;
            }
        }
        inline static size_t createSingleHash(const string & str) {
            size_t hash = 0;
            // cout << "Hash: " << "-  \t0" << endl;
            for (const char c : str) {
                hash *= MULTIPLY;
                hash += c - 'a';
                hash %= MODULO;
                // cout << "Hash: " << c << " \t" << hash << endl;
            }
            return hash;
        }
        // KMP implementation inspired by lectures and Wikipedia pseudocode
        NumSet searchString(const string & phrase) const {
            const size_t pLen = phrase.size();
            const size_t dLen = data.size();
            if (pLen > dLen) return NumSet();
            if (pLen == 0) return createAllIndexes(dLen);
            NumSet indexes;
            const vector<size_t> failFun = createFailFunction(phrase);

            size_t i = 0, j = 0;
            // size_t cycles = 0;
            while (i < dLen) {
                // cycles++;
                if (data[i] == phrase[j]) {
                    i++;
                    j++;
                    if (j >= pLen) {
                        indexes.emplace(i - j);
                        j = failFun[j];
                    }
                } else {
                    j = failFun[j];
                    if (j == (size_t)-1) {
                        i++;
                        j++;
                    }
                }
            }
            // cout << "Done " << cycles << " / " << data.size() << " - " << ((double) cycles / data.size()) << endl;

            return indexes;
        }

        inline NumSet createAllIndexes(const size_t len) const {
            NumSet s;
            for (size_t i = 0; i < len; i++)
                s.emplace(i);
            return s;
        }

        // computes fail function for the str given
        inline vector<size_t> createFailFunction(const string & str) const {
            const size_t len = str.size();
            vector<size_t> toReturn = { (size_t)-1, 0 };
            toReturn.resize(str.size() + 1);
            size_t pos = 0;

            for (size_t i = 1; i < len;) {
                if (str[i] == str[pos]) {
                    toReturn[++i] = ++pos;
                } else if (pos > 0) {
                    pos = toReturn[pos];
                } else {
                    pos = 0;
                    toReturn[++i] = 0;
                }
            }
            /*cout << ' ' << str << endl;
            for (auto i : toReturn)
                cout << i;
            cout << endl;*/

            return toReturn;
        }

        NumSet searchGeneral(const T & phrase) const {
            NumSet indexes;
            if (phrase.size() > data.size()) return indexes;

            for (auto itr = data.begin(); itr != data.end(); ++itr) {
                auto dItr = itr;
                auto pItr = phrase.begin();
                while (pItr != phrase.end()
                        && dItr != data.end()
                        && equal(*dItr, *pItr)) {
                    dItr++;
                    pItr++;
                }
                if (pItr == phrase.end()) indexes.emplace(distance(data.begin(), itr));
            }

            return indexes;
        }

        inline bool equal(const char v1, const char v2) const {
            if constexpr (is_same_v<C, less<char>>)
                return v1 == v2;
            else return equal<char>(v1, v2);
        }
        template<typename V>
            inline bool equal(const V & v1, const V & v2) const {
                return !cmp(v1, v2) && !cmp(v2, v1);
            }

        static size_t moduloPow(size_t what, size_t power, const size_t mod) {
            size_t res = 1;
            while (power > 0) {
                if (power & 1uL) {
                    res *= what;
                    res %= mod;
                }
                what *= what;
                what %= mod;
                power >>= 1;
            }
            return res;
        }
};


#ifndef __PROGTEST__
class CStrComparator {
    public:
        CStrComparator(bool caseSensitive )
            : m_CaseSensitive(caseSensitive){
            }
        bool operator ()(const string & a, const string & b)const {
            return m_CaseSensitive ? strcasecmp(a.c_str (), b.c_str ())< 0 : a < b ;
        }
        bool m_CaseSensitive;
};

bool upperCaseCompare(const char & a, const char & b){
    return toupper(a)< toupper(b );
}
void printSet(const NumSet & set) {
    cout << "> ";
    for (const auto item : set)
        cout << item << ", ";
    cout << endl;
}

int main(void){
    /*
    assert(moduloPow(2, 0, -1) ==  1); 
    assert(moduloPow(2, 1, -1) ==  2); 
    assert(moduloPow(2, 2, -1) ==  4); 
    assert(moduloPow(2, 3, -1) ==  8); 
    assert(moduloPow(2, 4, -1) == 16); 
    assert(moduloPow(2, 5, -1) == 32); 
    assert(moduloPow(2, 6, -1) == 64); 

    assert(moduloPow(2, 0, 13) ==  1); 
    assert(moduloPow(2, 1, 13) ==  2); 
    assert(moduloPow(2, 2, 13) ==  4); 
    assert(moduloPow(2, 3, 13) ==  8); 
    assert(moduloPow(2, 4, 13) ==  3); 
    assert(moduloPow(2, 5, 13) ==  6); 
    assert(moduloPow(2, 6, 13) == 12);
    */

    CIndex <string> i0("abcabcabc" );
    i0.search("cococay");
    i0.search("abcdabd");
    set<size_t> m0 = i0.search("abcabcabc" );
    assert( m0 ==(set<size_t> { 0 }));
    set<size_t> m1 = i0.search("abcabcab" );
    assert( m1 ==(set<size_t> { 0 }));
    set<size_t> m2 = i0.search("abcabca" );
    assert( m2 ==(set<size_t> { 0 }));
    set<size_t> m3 = i0.search("abcabcabca" );
    assert( m3 ==(set<size_t> { }));
    set<size_t> m4 = i0.search("abcabcabx" );
    assert( m4 ==(set<size_t> { }));

    CIndex <string> t0("abcabcabc" );
    set<size_t> r0 = t0.search("a" );
    assert( r0 ==(set<size_t> { 0, 3, 6 }));
    set<size_t> r1 = t0.search("abc" );
    assert( r1 ==(set<size_t> { 0, 3, 6 }));
    set<size_t> r2 = t0.search("abcabc" );
    assert( r2 ==(set<size_t> { 0, 3 }));

    CIndex <string> t1("abcababc" );
    set<size_t> r3 = t1.search("a" );
    assert( r3 ==(set<size_t> { 0, 3, 5 }));
    set<size_t> r4 = t1.search("abc" );
    assert( r4 ==(set<size_t> { 0, 5 }));
    set<size_t> r5 = t1.search("abcabc" );
    assert( r5 ==(set<size_t> { }));

    CIndex <string> t2("kokokokoskokosokos" );
    set<size_t> r6 = t2.search("kos" );
    assert( r6 ==(set<size_t> { 6, 11, 15 }));
    set<size_t> r7 = t2.search("kokos" );
    assert( r7 ==(set<size_t> { 4, 9 }));

    CIndex <string> t3("aaaaaaau aaauaaaau" );
    set<size_t> r8 = t3.search("aa" );
    assert( r8 ==(set<size_t> { 0, 1, 2, 3, 4, 5, 9, 10, 13, 14, 15 }));
    set<size_t> r9 = t3.search("aaa" );
    assert( r9 ==(set<size_t> { 0, 1, 2, 3, 4, 9, 13, 14 }));
    set<size_t> r10 = t3.search("aaaa" );
    assert( r10 ==(set<size_t> { 0, 1, 2, 3, 13 }));

    CIndex <string> t4("automatIc authentication automotive auTOmation raut" );
    set<size_t> r11 = t4.search("auto" );
    assert( r11 ==(set<size_t> { 0, 25 }));
    set<size_t> r12 = t4.search("aut" );
    assert( r12 ==(set<size_t> { 0, 10, 25, 48 }));
    set<size_t> r13 = t4.search("tic" );
    assert( r13 ==(set<size_t> { 16 }));
    set<size_t> r14 = t4.search("trunk" );
    assert( r14 ==(set<size_t> { }));
    set<size_t> r15 = t4.search("a" );
    assert( r15 ==(set<size_t> { 0, 5, 10, 19, 25, 36, 41, 48 }));
    set<size_t> r16 = t4.search("" );
    assert( r16 ==(set<size_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 }));

    CIndex <string, bool(*)(const char &, const char & )> t5("automatIc authentication automotive auTOmation raut", upperCaseCompare );
    set<size_t> r17 = t5.search("auto" );
    assert( r17 ==(set<size_t> { 0, 25, 36 }));
    set<size_t> r18 = t5.search("aut" );
    assert( r18 ==(set<size_t> { 0, 10, 25, 36, 48 }));
    set<size_t> r19 = t5.search("tic" );
    assert( r19 ==(set<size_t> { 6, 16 }));
    set<size_t> r20 = t5.search("trunk" );
    assert( r20 ==(set<size_t> { }));
    set<size_t> r21 = t5.search("a" );
    assert( r21 ==(set<size_t> { 0, 5, 10, 19, 25, 36, 41, 48 }));
    set<size_t> r22 = t5.search("" );
    assert( r22 ==(set<size_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 }));

    CIndex <list<string>, CStrComparator> t6(list<string>{ "Hello", "world", "test", "this", "foo", "TEsT", "this", "done" }, CStrComparator(false));
    set<size_t> r23 = t6.search(list<string>{"test", "this", "foo"} );
    assert( r23 ==(set<size_t> { 2 }));
    set<size_t> r24 = t6.search(list<string>{"test", "this"} );
    assert( r24 ==(set<size_t> { 2 }));

    CIndex <list<string>, CStrComparator> t7(list<string>{ "Hello", "world", "test", "this", "foo", "TEsT", "this", "done" }, CStrComparator(true));
    set<size_t> r25 = t7.search(list<string>{"test", "this", "foo"} );
    assert( r25 ==(set<size_t> { 2 }));
    set<size_t> r26 = t7.search(list<string>{"test", "this"} );
    assert( r26 ==(set<size_t> { 2, 5 }));

    cout << "All tests passed!" << endl;
#ifdef BLBOST
#endif
    return 0;
}
#endif /* __PROGTEST__ */
