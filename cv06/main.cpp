#ifndef __PROGTEST__
#include <cstring>
using namespace std;

class CLinkedSetTester;
#endif /* __PROGTEST__ */

class CLinkedSet {
    private:
        struct CNode {
            CNode * m_Next;
            const char * Value () const;
        };
        CNode * m_Begin;

    public:
        // default constructor
        // copy constructor
        // operator=
        // destructor

        bool Insert ( const char * value );
        bool Remove ( const char * value );
        bool Empty () const;
        size_t Size () const;
        bool Contains ( const char * value ) const;

        friend class ::CLinkedSetTester;
};

#ifndef __PROGTEST__
#include <cassert>

struct CLinkedSetTester {
    static void test0() {
        CLinkedSet x0;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert( !x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert( !x0.Remove( "Pavlik Jan" ) );
        assert( !x0.Empty() );
    }

    static void test1() {
        CLinkedSet x0;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert( !x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert( !x0.Remove( "Pavlik Jan" ) );
        CLinkedSet x1 ( x0 );
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
        assert( x0.Size() == 3 );
        assert( x1.Size() == 3 );
        assert( x0.Contains( "Vodickova Saskie" ) );
        assert( !x1.Contains( "Vodickova Saskie" ) );
        assert( !x0.Contains( "Kadlecova Kvetslava" ) );
        assert( x1.Contains( "Kadlecova Kvetslava" ) );
    }

    static void test2() {
        CLinkedSet x0;
        CLinkedSet x1;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert( !x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert( !x0.Remove( "Pavlik Jan" ) );
        x1 = x0;
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
        assert( x0.Size() == 3 );
        assert( x1.Size() == 3 );
        assert( x0.Contains( "Vodickova Saskie" ) );
        assert( !x1.Contains( "Vodickova Saskie" ) );
        assert( !x0.Contains( "Kadlecova Kvetslava" ) );
        assert( x1.Contains( "Kadlecova Kvetslava" ) );
    }
};

int main () {
    CLinkedSetTester::test0();
    CLinkedSetTester::test1();
    CLinkedSetTester::test2();
    return 0;
}
#endif /* __PROGTEST__ */
