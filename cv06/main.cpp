#ifndef __PROGTEST__
#include <cstring>
#include <iostream>
#include <iomanip>
using namespace std;

class CLinkedSetTester;
#endif /* __PROGTEST__ */

class CLinkedSet {
    private:
        struct CNode {
            CNode * m_Next = nullptr;
            char * m_Chars;

            CNode(const char * c) {
                const size_t len = strlen(c);
                m_Chars = new char[len + 1];
                for (size_t i = 0; i < len + 1; i++)
                    m_Chars[i] = c[i];
            }
            ~CNode() {
                delete [] m_Chars;
            }
            const char * Value () const { return m_Chars; }
        };

        CNode * m_Begin = nullptr;
        size_t m_Size = 0;
    public:
        CLinkedSet() {}
        CLinkedSet(const CLinkedSet & set) {
            CNode * node = set.m_Begin;
            CNode ** target = &m_Begin;
            while(node != nullptr) {
                *target = new CNode(node -> Value());
                target = &((*target) -> m_Next);
                node = node -> m_Next;
            }
            m_Size = set.m_Size;
        }
        //CLinkedSet(const CLinkedSet && set);
        ~CLinkedSet() {
            CNode * node = m_Begin;
            while (node != nullptr) {
                CNode * next = node -> m_Next;
                delete node;
                node = next;
            }
            m_Begin = nullptr;
            m_Size = 0;
        }
        CLinkedSet & operator=(CLinkedSet set) {
            {
                CNode * const tmp = m_Begin;
                m_Begin = set.m_Begin;
                set.m_Begin = tmp;
            }
            {
                const size_t tmp = m_Size;
                m_Size = set.m_Size;
                set.m_Size = tmp;
            }
            return *this;
        }

        bool Insert ( const char * value ) {
            if (m_Begin == nullptr) {
                m_Begin = new CNode(value);
                m_Size++;
                return true;
            }
            {
                int cmp = strcmp(value, m_Begin -> Value());
                if (cmp == 0) return false;
                if (cmp < 0) {
                    CNode * old = m_Begin;
                    m_Begin = new CNode(value);
                    m_Begin -> m_Next = old;
                    m_Size++;
                    return true;
                }
            }
            CNode * node = m_Begin -> m_Next;
            CNode ** prev = &m_Begin;
            while(node != nullptr) {
                int cmp = strcmp(value, node -> Value());
                if (cmp == 0) return false;
                if (cmp < 0) {
                    CNode * newNode = new CNode(value);
                    (*prev) -> m_Next = newNode;
                    newNode -> m_Next = node;
                    m_Size++;
                    return true;
                }
                prev = &((*prev) -> m_Next);
                node = node -> m_Next;
            }
            (*prev) -> m_Next = new CNode(value);
            m_Size++;
            return true;
        }
        bool Remove ( const char * value ) {
            if (m_Size == 0) return false;
            {
                int cmp = strcmp(value, m_Begin -> Value());
                if (cmp == 0) {
                    CNode * next = m_Begin -> m_Next;
                    delete m_Begin;
                    m_Begin = next;
                    m_Size--;
                    return true;
                }
            }
            CNode * node = m_Begin -> m_Next;
            CNode ** prev = &m_Begin;
            while(node != nullptr) {
                int cmp = strcmp(value, node -> Value());
                // cout << "Comparing P: " << (*prev) -> Value() << " and N: " << node -> Value() << endl;
                if (cmp == 0) {
                    (*prev) -> m_Next = node -> m_Next;
                    delete node;
                    m_Size--;
                    return true;
                }
                prev = &((*prev) -> m_Next);
                node = node -> m_Next;
            }
            return false;
        }
        bool Contains ( const char * value ) const {
            CNode * node = m_Begin;
            while (node != nullptr) {
                if (strcmp(value, node -> Value()) == 0)
                    return true;
                node = node -> m_Next;
            }
            return false;
        }
        bool Empty () const { return Size() == 0; }
        size_t Size () const { return m_Size; }

        friend class ::CLinkedSetTester;
        friend void printLinkedSet(const CLinkedSet & set);
};

#ifndef __PROGTEST__
#include <cassert>

void printLinkedSet(const CLinkedSet & set) {
    CLinkedSet::CNode * node = set.m_Begin;
    while (node) {
        cout << node -> Value() << endl;
        node = node -> m_Next;
    }
    cout << setw(42) << setfill('-') << "" << endl;
}

struct CLinkedSetTester {
    static void mTest0() {
        CLinkedSet x0;
        assert( x0.Insert( "1" ) );
        assert( x0.Insert( "2" ) );
        assert( x0.Insert( "3" ) );
        assert( x0.Insert( "4" ) );
        assert( x0.Insert( "5" ) );
        assert( x0.Remove( "1" ) );
        printLinkedSet(x0);
    }
    static void mTest1() {
        CLinkedSet x0;
        assert( x0.Insert( "1" ) );
        assert( x0.Insert( "2" ) );
        assert( x0.Insert( "3" ) );
        assert( x0.Insert( "4" ) );
        assert( x0.Insert( "5" ) );
        assert( x0.Remove( "2" ) );
        printLinkedSet(x0);
    }
    static void mTest2() {
        CLinkedSet x0;
        assert( x0.Insert( "1" ) );
        assert( x0.Insert( "2" ) );
        assert( x0.Insert( "3" ) );
        assert( x0.Insert( "4" ) );
        assert( x0.Insert( "5" ) );
        assert( x0.Remove( "3" ) );
        printLinkedSet(x0);
    }
    static void mTest3() {
        CLinkedSet x0;
        assert( x0.Insert( "1" ) );
        assert( x0.Insert( "2" ) );
        assert( x0.Insert( "3" ) );
        assert( x0.Insert( "4" ) );
        assert( x0.Insert( "5" ) );
        assert( x0.Remove( "4" ) );
        printLinkedSet(x0);
    }
    static void mTest4() {
        CLinkedSet x0;
        assert( x0.Insert( "1" ) );
        assert( x0.Insert( "2" ) );
        assert( x0.Insert( "3" ) );
        assert( x0.Insert( "4" ) );
        assert( x0.Insert( "5" ) );
        assert( x0.Remove( "5" ) );
        printLinkedSet(x0);
    }

    static void test0() {
        CLinkedSet x0;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        assert(!x0.Empty() );
        printLinkedSet(x0);
    }
    static void test1() {
        CLinkedSet x0;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        printLinkedSet(x0);
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        CLinkedSet x1 ( x0 );
        printLinkedSet(x0);
        printLinkedSet(x1);
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
        printLinkedSet(x0);
        printLinkedSet(x1);
        assert( x0.Size() == 3 );
        assert( x1.Size() == 3 );
        assert( x0.Contains( "Vodickova Saskie" ) );
        assert(!x1.Contains( "Vodickova Saskie" ) );
        assert(!x0.Contains( "Kadlecova Kvetslava" ) );
        assert( x1.Contains( "Kadlecova Kvetslava" ) );
    }
    static void test2() {
        CLinkedSet x0;
        CLinkedSet x1;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        printLinkedSet(x0);
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        x1 = x0;
        printLinkedSet(x0);
        printLinkedSet(x1);
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
        printLinkedSet(x0);
        printLinkedSet(x1);
        assert( x0.Size() == 3 );
        assert( x1.Size() == 3 );
        assert( x0.Contains( "Vodickova Saskie" ) );
        assert(!x1.Contains( "Vodickova Saskie" ) );
        assert(!x0.Contains( "Kadlecova Kvetslava" ) );
        assert( x1.Contains( "Kadlecova Kvetslava" ) );
    }
};

int main () {
    CLinkedSetTester::mTest0();
    CLinkedSetTester::mTest1();
    CLinkedSetTester::mTest2();
    CLinkedSetTester::mTest3();
    CLinkedSetTester::mTest4();

    CLinkedSetTester::test0();
    CLinkedSetTester::test1();
    CLinkedSetTester::test2();
    return 0;
}
#endif /* __PROGTEST__ */
