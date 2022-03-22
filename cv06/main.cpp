#ifndef __PROGTEST__
#include <cstring>
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
        CLinkedSet(const CLinkedSet &  set) {
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
        }
        CLinkedSet & operator=(CLinkedSet set) {
            {
                CNode * tmp = m_Begin;
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
                int cmp = strcmp(m_Begin -> Value(), value);
                if (cmp == 0) return false;
                if (cmp > 0) {
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
                int cmp = strcmp(node -> Value(), value);
                if (cmp == 0) return false;
                if (cmp > 0) {
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
            if (m_Size == 1) {
                int cmp = strcmp(m_Begin -> Value(), value);
                if (cmp != 0) return false;
                delete m_Begin;
                m_Begin = nullptr;
                m_Size--;
                return true;
            }
            CNode * node = m_Begin -> m_Next;
            CNode ** prev = &m_Begin;
            while(node != nullptr) {
                int cmp = strcmp(node -> Value(), value);
                if (cmp == 0) {
                    CNode * next = node -> m_Next;
                    (*prev) -> m_Next = next;
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
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        assert(!x0.Empty() );
    }

    static void test1() {
        CLinkedSet x0;
        assert( x0.Insert( "Jerabek Michal" ) );
        assert( x0.Insert( "Pavlik Ales" ) );
        assert( x0.Insert( "Dusek Zikmund" ) );
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        CLinkedSet x1 ( x0 );
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
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
        assert( x0.Size() == 3 );
        assert( x0.Contains( "Dusek Zikmund" ) );
        assert(!x0.Contains( "Pavlik Jan" ) );
        assert( x0.Remove( "Jerabek Michal" ) );
        assert(!x0.Remove( "Pavlik Jan" ) );
        x1 = x0;
        assert( x0.Insert( "Vodickova Saskie" ) );
        assert( x1.Insert( "Kadlecova Kvetslava" ) );
        assert( x0.Size() == 3 );
        assert( x1.Size() == 3 );
        assert( x0.Contains( "Vodickova Saskie" ) );
        assert(!x1.Contains( "Vodickova Saskie" ) );
        assert(!x0.Contains( "Kadlecova Kvetslava" ) );
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
