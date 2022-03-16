#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
using namespace std;

class CTimeTester;
#endif /* __PROGTEST__ */

class CTime {
    private:
        int m_Hour;
        int m_Minute;
        int m_Second;

    public:
        CTime();
        explicit CTime(const int hour, const int minute, const int second);

        int operator - (const CTime & other) const;

        CTime operator +  (const int seconds) const;
        CTime operator -  (const int seconds) const;
        friend CTime operator + (const int seconds, const CTime & time);
        friend CTime operator - (const int seconds, const CTime & time);

        CTime & operator += (const int seconds);
        CTime & operator -= (const int seconds);
        CTime operator ++ (int);
        CTime operator -- (int);
        CTime & operator ++ ();
        CTime & operator -- ();

        bool operator <  (const CTime & other) const;
        bool operator >  (const CTime & other) const;
        bool operator <= (const CTime & other) const;
        bool operator >= (const CTime & other) const;
        bool operator == (const CTime & other) const;
        bool operator != (const CTime & other) const;

        friend ostream & operator << (ostream & out, const CTime & time);

        friend class ::CTimeTester;
};

CTime::CTime(): CTime(0, 0, 0) {}
CTime::CTime(const int hour, const int minute, const int second = 0)
    : m_Hour(hour), m_Minute(minute), m_Second(second) {
        if (hour < 0 || 24 <= hour)
            throw invalid_argument("Hours out of bounds");
        if (minute < 0 || 60 <= minute)
            throw invalid_argument("Minutes out of bounds");
        if (second < 0 || 60 <= second)
            throw invalid_argument("Seconds out of bounds");
}

int CTime::operator - (const CTime & other) const {
    int sec1 = m_Hour * 3600 + m_Minute * 60 + m_Second;
    int sec2 = other.m_Hour * 3600 + other.m_Minute * 60 + other.m_Second;
    return min(min(abs(sec1 - sec2), abs(24 * 3600 + sec1 - sec2)), abs(24 * 3600 + sec2 - sec1));
}

CTime CTime::operator +  (const int plusSeconds) const {
    int second = m_Second + plusSeconds;
    int minute = m_Minute + second / 60 + (second % 60 >= 0 ? 0 : -1);
    int hour = m_Hour + minute / 60 + (minute % 60 >= 0 ? 0 : -1);
    hour =  hour % 24 + (hour % 24 >= 0 ? 0 : 24);
    minute =  minute % 60 + (minute % 60 >= 0 ? 0 : 60);
    second =  second % 60 + (second % 60 >= 0 ? 0 : 60);
    return CTime(hour, minute, second);
}
inline CTime CTime::operator - (const int seconds) const {
    return (*this) + (-seconds);
}
CTime operator + (const int seconds, const CTime & time) {
    return time + seconds;
}
CTime operator - (const int seconds, const CTime & time) {
    return time - seconds;
}

CTime & CTime::operator += (const int seconds) {
    const CTime added = (*this) + seconds;
    m_Hour    = added.m_Hour; 
    m_Minute  = added.m_Minute;
    m_Second  = added.m_Second;
    return *this;
}
inline CTime & CTime::operator -= (const int seconds) { return (*this) += -seconds; }
inline CTime CTime::operator ++ (int) {
    CTime copy = *this;
    (*this) += 1;
    return copy;
}
inline CTime CTime::operator -- (int) {
    CTime copy = *this;
    (*this) -= 1;
    return copy;
}
inline CTime & CTime::operator ++ () { return (*this) += 1; }
inline CTime & CTime::operator -- () { return (*this) -= 1; }

bool CTime::operator <  (const CTime & other) const {
    if (m_Hour   < other.m_Hour  ) return true;
    if (m_Hour   > other.m_Hour  ) return false;
    if (m_Minute < other.m_Minute) return true;
    if (m_Minute > other.m_Minute) return false;
    if (m_Second < other.m_Second) return true;
    if (m_Second > other.m_Second) return false;
    return false;
}
bool CTime::operator >  (const CTime & other) const {
    if (m_Hour   > other.m_Hour  ) return true;
    if (m_Hour   < other.m_Hour  ) return false;
    if (m_Minute > other.m_Minute) return true;
    if (m_Minute < other.m_Minute) return false;
    if (m_Second > other.m_Second) return true;
    if (m_Second < other.m_Second) return false;
    return false;
}
inline bool CTime::operator <= (const CTime & other) const {
    return !((*this) > other);
}
inline bool CTime::operator >= (const CTime & other) const {
    return !((*this) < other);
}
bool CTime::operator == (const CTime & other) const {
        return m_Hour == other.m_Hour
            && m_Minute == other.m_Minute
            && m_Second == other.m_Second;
}
inline bool CTime::operator != (const CTime & other) const {
    return !((*this) == other);
}

ostream & operator << (ostream & out, const CTime & time) {
    out
        << setw(2) << setfill(' ')
        << time.m_Hour << ":"
        << setw(2) << setfill('0')
        << time.m_Minute << ":"
        << setw(2) << setfill('0')
        << time.m_Second;
    return out;
}



#ifndef __PROGTEST__
struct CTimeTester {
    static bool test ( const CTime & time, int hour, int minute, int second ) {
        cout << "Testing " << time << " vs " << CTime(hour, minute, second) << endl;
        return time.m_Hour == hour
            && time.m_Minute == minute
            && time.m_Second == second;
    }
};

#include <cassert>
#include <sstream>

int main () {

    const int daySec = 24 * 3600;

    CTime mid;
    assert( CTimeTester::test( mid - 1, 23, 59, 59 ) );
    assert( CTimeTester::test( mid - (1 + daySec) , 23, 59, 59 ) );
    assert( CTimeTester::test( mid -= 1, 23, 59, 59 ) );
    assert( CTimeTester::test( mid += 1, 0, 0, 0 ) );
    assert( CTimeTester::test( mid -= 1 + daySec, 23, 59, 59 ) );

    CTime a { 12, 30 };
    assert( CTimeTester::test( a, 12, 30, 0 ) );

    CTime b { 13, 30 };
    assert( CTimeTester::test( b, 13, 30, 0 ) );

    mid = {};
    assert( mid - (mid + 1) == 1 );
    assert( (mid + 1) - mid == 1 );
    assert( mid - (mid - 1) == 1 );
    assert( (mid - 1) - mid == 1 );

    assert( CTimeTester::test( a + 60, 12, 31, 0 ) );
    assert( CTimeTester::test( a - 60, 12, 29, 0 ) );
    assert( CTimeTester::test( a + 61, 12, 31, 1 ) );
    assert( CTimeTester::test( a - 61, 12, 28, 59 ) );
    assert( CTimeTester::test( a + 59, 12, 30, 59 ) );
    assert( CTimeTester::test( a - 59, 12, 29, 1 ) );
    assert( CTimeTester::test( a + 120, 12, 32, 0 ) );
    assert( CTimeTester::test( a - 120, 12, 28, 0 ) );
    assert( CTimeTester::test( a + 121, 12, 32, 1 ) );
    assert( CTimeTester::test( a - 121, 12, 27, 59 ) );
    assert( CTimeTester::test( a + 119, 12, 31, 59 ) );
    assert( CTimeTester::test( a - 119, 12, 28, 1 ) );
    assert( CTimeTester::test( a + 0, 12, 30, 0 ) );
    assert( CTimeTester::test( a - 0, 12, 30, 0 ) );

    assert( CTimeTester::test( 60 + a, 12, 31, 0 ) );
    assert( CTimeTester::test( 60 - a, 12, 29, 0 ) );

    assert( a < b );
    assert( a <= b );
    assert( a != b );
    assert( !( a > b ) );
    assert( !( a >= b ) );
    assert( !( a == b ) );

    while ( ++a != b );
    assert( a == b );

    std::ostringstream output;
    assert( static_cast<bool>( output << a ) );
    assert( output.str() == "13:30:00" );

    assert( a++ == b++ );
    assert( a == b );

    assert( --a == --b );
    assert( a == b );

    assert( a-- == b-- );
    assert( a == b );

    cout << "Tests PASSED!" << endl;
    return 0;
}
#endif /* __PROGTEST__ */
