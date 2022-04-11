#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <algorithm>
#include <functional>

#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

template<typename T>
class CSet {
    private:
        struct CNode {
            CNode * mNext = nullptr;
            T mValue;

            CNode(const T & val) : mValue(val) {}
            ~CNode() = default;
            T& Value() { return mValue; }
            const T& Value() const { return mValue; }
        };

        CNode * mBegin = nullptr;
        size_t mSize = 0;
    public:
        CSet() {}
        CSet(const CSet & set) {
            CNode * node = set.mBegin;
            CNode ** target = &mBegin;
            while(node != nullptr) {
                *target = new CNode(node -> Value());
                target = &((*target) -> mNext);
                node = node -> mNext;
            }
            mSize = set.mSize;
        }
        //CSet(const CSet && set);
        ~CSet() {
            CNode * node = mBegin;
            while (node != nullptr) {
                CNode * next = node -> mNext;
                delete node;
                node = next;
            }
            mBegin = nullptr;
            mSize = 0;
        }
        CSet & operator=(CSet set) {
            {
                CNode * const tmp = mBegin;
                mBegin = set.mBegin;
                set.mBegin = tmp;
            }
            {
                const size_t tmp = mSize;
                mSize = set.mSize;
                set.mSize = tmp;
            }
            return *this;
        }

        bool Insert ( const T & value ) {
            if (mBegin == nullptr) {
                mBegin = new CNode(value);
                mSize++;
                return true;
            }
            {
                int cmp = itemCmp(value, mBegin -> Value());
                if (cmp == 0) return false;
                if (cmp < 0) {
                    CNode * old = mBegin;
                    mBegin = new CNode(value);
                    mBegin -> mNext = old;
                    mSize++;
                    return true;
                }
            }
            CNode * node = mBegin -> mNext;
            CNode ** prev = &mBegin;
            while(node != nullptr) {
                int cmp = itemCmp(value, node -> Value());
                if (cmp == 0) return false;
                if (cmp < 0) {
                    CNode * newNode = new CNode(value);
                    (*prev) -> mNext = newNode;
                    newNode -> mNext = node;
                    mSize++;
                    return true;
                }
                prev = &((*prev) -> mNext);
                node = node -> mNext;
            }
            (*prev) -> mNext = new CNode(value);
            mSize++;
            return true;
        }
        bool Remove ( const T& value ) {
            if (mSize == 0) return false;
            {
                int cmp = itemCmp(value, mBegin -> Value());
                if (cmp == 0) {
                    CNode * next = mBegin -> mNext;
                    delete mBegin;
                    mBegin = next;
                    mSize--;
                    return true;
                }
            }
            CNode * node = mBegin -> mNext;
            CNode ** prev = &mBegin;
            while(node != nullptr) {
                int cmp = itemCmp(value, node -> Value());
                if (cmp == 0) {
                    (*prev) -> mNext = node -> mNext;
                    delete node;
                    mSize--;
                    return true;
                }
                prev = &((*prev) -> mNext);
                node = node -> mNext;
            }
            return false;
        }
        bool Contains ( const T& value ) const {
            CNode * node = mBegin;
            while (node != nullptr) {
                if (itemCmp(value, node -> Value()) == 0)
                    return true;
                node = node -> mNext;
            }
            return false;
        }
        bool Empty () const { return Size() == 0; }
        size_t Size () const { return mSize; }
    private:
        int itemCmp(const T& i1, const T& i2) const {
            if (i1 < i2) return -1;
            if (i2 < i1) return  1;
            return 0;
        }
};

#ifndef __PROGTEST__
#include <cassert>

struct CSetTester {
    static void test0() {
        CSet<string> x0;
        assert( x0.Insert( "Jerabek Michal" ));
        assert( x0.Insert( "Pavlik Ales" ));
        assert( x0.Insert( "Dusek Zikmund" ));
        assert( x0.Size() == 3);
        assert( x0.Contains( "Dusek Zikmund" ));
        assert( !x0.Contains( "Pavlik Jan" ));
        assert( x0.Remove( "Jerabek Michal" ));
        assert( !x0.Remove( "Pavlik Jan" ));
    }
    static void test1() {
        CSet<string> x0;
        assert( x0.Insert( "Jerabek Michal" ));
        assert( x0.Insert( "Pavlik Ales" ));
        assert( x0.Insert( "Dusek Zikmund" ));
        assert( x0.Size() == 3);
        assert( x0.Contains( "Dusek Zikmund" ));
        assert( !x0.Contains( "Pavlik Jan" ));
        assert( x0.Remove( "Jerabek Michal" ));
        assert( !x0.Remove( "Pavlik Jan" ));
        CSet<string> x1(x0);
        assert( x0.Insert( "Vodickova Saskie" ));
        assert( x1.Insert( "Kadlecova Kvetslava" ));
        assert( x0.Size() == 3);
        assert( x1.Size() == 3);
        assert( x0.Contains( "Vodickova Saskie" ));
        assert( !x1.Contains( "Vodickova Saskie" ));
        assert( !x0.Contains( "Kadlecova Kvetslava" ));
        assert( x1.Contains( "Kadlecova Kvetslava" ));
    }
    static void test2() {
        CSet<string> x0;
        CSet<string> x1;
        assert( x0.Insert( "Jerabek Michal" ));
        assert( x0.Insert( "Pavlik Ales" ));
        assert( x0.Insert( "Dusek Zikmund" ));
        assert( x0.Size() == 3);
        assert( x0.Contains( "Dusek Zikmund" ));
        assert( !x0.Contains( "Pavlik Jan" ));
        assert( x0.Remove( "Jerabek Michal" ));
        assert( !x0.Remove( "Pavlik Jan" ));
        x1 = x0;
        assert( x0.Insert( "Vodickova Saskie" ));
        assert( x1.Insert( "Kadlecova Kvetslava" ));
        assert( x0.Size() == 3);
        assert( x1.Size() == 3);
        assert( x0.Contains( "Vodickova Saskie" ));
        assert( !x1.Contains( "Vodickova Saskie" ));
        assert( !x0.Contains( "Kadlecova Kvetslava" ));
        assert( x1.Contains( "Kadlecova Kvetslava" ));
    }
    static void test4() {
        CSet<int> x0;
        assert( x0.Insert( 4 ));
        assert( x0.Insert( 8 ));
        assert( x0.Insert( 1 ));
        assert( x0.Size() == 3);
        assert( x0.Contains( 4 ));
        assert( !x0.Contains( 5 ));
        assert( !x0.Remove( 5 ));
        assert( x0.Remove( 4 ));
    }
};

int main () {
    CSetTester::test0();
    CSetTester::test1();
    CSetTester::test2();
    CSetTester::test4();
    cout << "All tests passed" << endl;
    return 0;
}
#endif /* __PROGTEST__ */
