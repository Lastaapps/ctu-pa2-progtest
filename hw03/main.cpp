#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
using namespace std;
#endif /* __PROGTEST__ */

// a dummy exception class, keep this implementation
class InvalidDateException : public invalid_argument {
    public:
        InvalidDateException ( )
            : invalid_argument ( "invalid date or format" ) {}
};
// date_format manipulator
ios_base & ( * date_format ( const char * fmt ) ) ( ios_base & x ) {
    return [] ( ios_base & ios ) -> ios_base & { return ios; };
}


class CDate {
    private:
        const uint8_t JANUARY = 1;
        int mYear;
        int mMonth;
        int mDay;
    public:
        CDate(const int year, const int month, const int day);
        CDate operator +(const int days) const;
        CDate operator -(const int days) const;
        int operator-(const CDate & date) const;
        CDate operator++(int);
        CDate operator--(int);
        CDate & operator++();
        CDate & operator--();
        bool operator < (const CDate & date) const;
        bool operator > (const CDate & date) const;
        bool operator <=(const CDate & date) const;
        bool operator >=(const CDate & date) const;
        bool operator ==(const CDate & date) const;
        bool operator !=(const CDate & date) const;
        friend ostream & operator << (ostream & out, const CDate & date);
        friend istream & operator >> (istream & in, CDate & date);
    private:
        static bool checkValid(const int year, const int month, const int day);
        static int daysInMonth(const int month, const bool isLeap);
        static bool isLeap(const int year);
        static int leapInInterval(int start, int end);

        friend void testIsLeap();
        friend void testLeapInInterval();
};

CDate::CDate(int year, int month, int day)
    : mYear(year), mMonth(month), mDay(day) {
        if (!checkValid(year, month, day))
            throw "InvalidDateException";
    }

bool CDate::checkValid(const int year, const int month, const int day) {
    if (month < 1 || 12 < month) return false;
    if (day < 1 || CDate::daysInMonth(month, isLeap(year)) < day) return false;
    return true;
}
bool CDate::isLeap(const int year) {
    return (year % 400 == 0 || year % 100 != 0) && year % 4 == 0;
}
int CDate::leapInInterval(int start, int end) {
	start -= 1; end -= 1;
	return 0 + (end / 400 - start / 400)
		- (end / 100 - start / 100)
		+ (end / 4 - start / 4);
}
int CDate::daysInMonth(const int month, const bool isLeap) {
    switch (month) {
        case  1: return 31;
        case  2: return isLeap ? 29 : 28;
        case  3: return 31;
        case  4: return 30;
        case  5: return 31;
        case  6: return 30;
        case  7: return 31;
        case  8: return 31;
        case  9: return 30;
        case 10: return 31;
        case 11: return 30;
        case 12: return 31;
        default: throw "Unknown month";
    }
}

bool CDate::operator < (const CDate & date) const {
    if (mYear  < date.mYear ) return true;
    if (mYear  > date.mYear ) return false;
    if (mMonth < date.mMonth) return true;
    if (mMonth > date.mMonth) return false;
    if (mDay   < date.mDay  ) return true;
    if (mDay   > date.mDay  ) return false;
    return false;
}
inline bool CDate::operator > (const CDate & date) const {
    return !(*this <= date);
}
bool CDate::operator <=(const CDate & date) const {
    if (mYear  < date.mYear ) return true;
    if (mYear  > date.mYear ) return false;
    if (mMonth < date.mMonth) return true;
    if (mMonth > date.mMonth) return false;
    if (mDay   < date.mDay  ) return true;
    if (mDay   > date.mDay  ) return false;
    return true;
}
inline bool CDate::operator >=(const CDate & date) const {
    return !(*this < date);
}
bool CDate::operator ==(const CDate & date) const {
    if (mYear  != date.mYear ) return false;
    if (mMonth != date.mMonth) return false;
    if (mDay   != date.mDay  ) return false;
    return true;
}
inline bool CDate::operator !=(const CDate & date) const {
    return !(*this == date);
}
ostream & operator << (ostream & out, const CDate & date) {
    return out
        << setw(4) << setfill('0') << date.mYear << '-'
        << setw(2) << setfill('0') << date.mMonth << '-'
        << setw(2) << setfill('0') << date.mDay << endl;
}
istream & operator >> (istream & in, CDate & date) {
    int year, month, day;
    char dash;
    in >> year;
    in >> dash;
    if (dash != '-') {
        in.setstate(std::ios::failbit);
        return in;
    }
    in >> month;
    in >> dash;
    if (dash != '-') {
        in.setstate(std::ios::failbit);
        return in;
    }
    in >> day;
    if (!CDate::checkValid(year, month, day)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    date.mYear  = year;
    date.mMonth = month;
    date.mDay   = day;
    return in;
}

#ifndef __PROGTEST__
void testIsLeap() {
	assert(CDate::isLeap(2001) == false);
	assert(CDate::isLeap(2004) == true);
	assert(CDate::isLeap(2100) == false);
	assert(CDate::isLeap(2400) == true);
}
void testLeapInInterval() {
	assert(CDate::leapInInterval(1999, 2011) == 3);
	assert(CDate::leapInInterval(2000, 2011) == 3);
	assert(CDate::leapInInterval(2001, 2011) == 2);
	assert(CDate::leapInInterval(2000, 2012) == 3);
	assert(CDate::leapInInterval(2000, 2013) == 4);
	assert(CDate::leapInInterval(2000, 2013) == 4);
	assert(CDate::leapInInterval(2099, 2101) == 0);
	assert(CDate::leapInInterval(2399, 2401) == 1);
	assert(CDate::leapInInterval(1818, 6969) == 1250);
}

void testProgrest() {

#ifdef ready
    ostringstream oss;
    istringstream iss;

    CDate a ( 2000, 1, 2 );
    CDate b ( 2010, 2, 3 );
    CDate c ( 2004, 2, 10 );
    oss.str ("");
    oss << a;
    assert ( oss.str () == "2000-01-02" );
    oss.str ("");
    oss << b;
    assert ( oss.str () == "2010-02-03" );
    oss.str ("");
    oss << c;
    assert ( oss.str () == "2004-02-10" );
    a = a + 1500;
    oss.str ("");
    oss << a;
    assert ( oss.str () == "2004-02-10" );
    b = b - 2000;
    oss.str ("");
    oss << b;
    assert ( oss.str () == "2004-08-13" );
    assert ( b - a == 185 );
    assert ( ( b == a ) == false );
    assert ( ( b != a ) == true );
    assert ( ( b <= a ) == false );
    assert ( ( b < a ) == false );
    assert ( ( b >= a ) == true );
    assert ( ( b > a ) == true );
    assert ( ( c == a ) == true );
    assert ( ( c != a ) == false );
    assert ( ( c <= a ) == true );
    assert ( ( c < a ) == false );
    assert ( ( c >= a ) == true );
    assert ( ( c > a ) == false );
    a = ++c;
    oss.str ( "" );
    oss << a << " " << c;
    assert ( oss.str () == "2004-02-11 2004-02-11" );
    a = --c;
    oss.str ( "" );
    oss << a << " " << c;
    assert ( oss.str () == "2004-02-10 2004-02-10" );
    a = c++;
    oss.str ( "" );
    oss << a << " " << c;
    assert ( oss.str () == "2004-02-10 2004-02-11" );
    a = c--;
    oss.str ( "" );
    oss << a << " " << c;
    assert ( oss.str () == "2004-02-11 2004-02-10" );
    iss.clear ();
    iss.str ( "2015-09-03" );
    assert ( ( iss >> a ) );
    oss.str ("");
    oss << a;
    assert ( oss.str () == "2015-09-03" );
    a = a + 70;
    oss.str ("");
    oss << a;
    assert ( oss.str () == "2015-11-12" );

    CDate d ( 2000, 1, 1 );
    try {
        CDate e ( 2000, 32, 1 );
        assert ( "No exception thrown!" == nullptr );
    } catch ( ... ) {
    }
    iss.clear ();
    iss.str ( "2000-12-33" );
    assert ( ! ( iss >> d ) );
    oss.str ("");
    oss << d;
    assert ( oss.str () == "2000-01-01" );
    iss.clear ();
    iss.str ( "2000-11-31" );
    assert ( ! ( iss >> d ) );
    oss.str ("");
    oss << d;
    assert ( oss.str () == "2000-01-01" );
    iss.clear ();
    iss.str ( "2000-02-29" );
    assert ( ( iss >> d ) );
    oss.str ("");
    oss << d;
    assert ( oss.str () == "2000-02-29" );
    iss.clear ();
    iss.str ( "2001-02-29" );
    assert ( ! ( iss >> d ) );
    oss.str ("");
    oss << d;
    assert ( oss.str () == "2000-02-29" );

    //-----------------------------------------------------------------------------
    // bonus test examples
    //-----------------------------------------------------------------------------
    CDate f ( 2000, 5, 12 );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2000-05-12" );
    oss.str ("");
    oss << date_format ( "%Y/%m/%d" ) << f;
    assert ( oss.str () == "2000/05/12" );
    oss.str ("");
    oss << date_format ( "%d.%m.%Y" ) << f;
    assert ( oss.str () == "12.05.2000" );
    oss.str ("");
    oss << date_format ( "%m/%d/%Y" ) << f;
    assert ( oss.str () == "05/12/2000" );
    oss.str ("");
    oss << date_format ( "%Y%m%d" ) << f;
    assert ( oss.str () == "20000512" );
    oss.str ("");
    oss << date_format ( "hello kitty" ) << f;
    assert ( oss.str () == "hello kitty" );
    oss.str ("");
    oss << date_format ( "%d%d%d%d%d%d%m%m%m%Y%Y%Y%%%%%%%%%%" ) << f;
    assert ( oss.str () == "121212121212050505200020002000%%%%%" );
    oss.str ("");
    oss << date_format ( "%Y-%m-%d" ) << f;
    assert ( oss.str () == "2000-05-12" );
    iss.clear ();
    iss.str ( "2001-01-1" );
    assert ( ! ( iss >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2000-05-12" );
    iss.clear ();
    iss.str ( "2001-1-01" );
    assert ( ! ( iss >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2000-05-12" );
    iss.clear ();
    iss.str ( "2001-001-01" );
    assert ( ! ( iss >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2000-05-12" );
    iss.clear ();
    iss.str ( "2001-01-02" );
    assert ( ( iss >> date_format ( "%Y-%m-%d" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2001-01-02" );
    iss.clear ();
    iss.str ( "05.06.2003" );
    assert ( ( iss >> date_format ( "%d.%m.%Y" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2003-06-05" );
    iss.clear ();
    iss.str ( "07/08/2004" );
    assert ( ( iss >> date_format ( "%m/%d/%Y" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2004-07-08" );
    iss.clear ();
    iss.str ( "2002*03*04" );
    assert ( ( iss >> date_format ( "%Y*%m*%d" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2002-03-04" );
    iss.clear ();
    iss.str ( "C++09format10PA22006rulez" );
    assert ( ( iss >> date_format ( "C++%mformat%dPA2%Yrulez" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2006-09-10" );
    iss.clear ();
    iss.str ( "%12%13%2010%" );
    assert ( ( iss >> date_format ( "%%%m%%%d%%%Y%%" ) >> f ) );
    oss.str ("");
    oss << f;
    assert ( oss.str () == "2010-12-13" );

    CDate g ( 2000, 6, 8 );
    iss.clear ();
    iss.str ( "2001-11-33" );
    assert ( ! ( iss >> date_format ( "%Y-%m-%d" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "29.02.2003" );
    assert ( ! ( iss >> date_format ( "%d.%m.%Y" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "14/02/2004" );
    assert ( ! ( iss >> date_format ( "%m/%d/%Y" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "2002-03" );
    assert ( ! ( iss >> date_format ( "%Y-%m" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "hello kitty" );
    assert ( ! ( iss >> date_format ( "hello kitty" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "2005-07-12-07" );
    assert ( ! ( iss >> date_format ( "%Y-%m-%d-%m" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-06-08" );
    iss.clear ();
    iss.str ( "20000101" );
    assert ( ( iss >> date_format ( "%Y%m%d" ) >> g ) );
    oss.str ("");
    oss << g;
    assert ( oss.str () == "2000-01-01" );
#endif
}

int main ( void ) {

    testIsLeap();
    testLeapInInterval();
    testProgrest();
    cout << "All tests PASSED!" << endl;

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
