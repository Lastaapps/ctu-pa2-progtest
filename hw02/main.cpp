#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
using namespace std;
#endif /* __PROGTEST__ */

class CVATRegister {
    public:
        CVATRegister ( void );
        ~CVATRegister ( void );

        bool newCompany ( const string & name, const string & addr, const string & taxID );

        bool cancelCompany ( const string & name, const string & addr );
        bool cancelCompany ( const string & taxID );

        bool invoice ( const string & taxID, unsigned int amount );
        bool invoice ( const string & name, const string & addr, unsigned int amount );

        bool audit ( const string & name, const string & addr, unsigned int & sumIncome ) const;
        bool audit ( const string & taxID, unsigned int & sumIncome ) const;

        unsigned int medianInvoice ( void ) const;

        bool firstCompany ( string & name, string & addr ) const;
        bool nextCompany ( string & name, string & addr ) const;

    class Company {
        private:
            const string & mName;
            const string & mAddr;
            const string & mId;
            unsigned int mAmount;
        public:
            Company(const string & name, const string & addr,
                    const string & id, unsigned int amount = 0)
                : mName(name), mAddr(addr), mId(id), mAmount(amount) {}

        class CompareNameAddr {
            private:
                inline char normalizeChar(const char c) const {
                    return ('a' <= c && c <= 'z') ? c : c - ('A' - 'a');
                }
            public:
                inline bool operator () (const string & s1, const string & s2) const {
                    size_t length = min(s1.length(), s2.length());
                    for (size_t i = 0; i < length; i++) {
                        if (normalizeChar(s1.at(i)) < normalizeChar(s2.at(i))) {
                            return true;
                        }
                    }
                    return s1.length() < s2.length();
                }
                inline bool operator() (const Company & c1, const Company & c2) const {
                    if ((*this)(c1.mName, c2.mName)) return true;
                    if ((*this)(c1.mAddr, c2.mAddr)) return true;
                    return false;
                }
        };
        class CompareIdPtr {
            public:
                inline bool operator() (const Company * c1, const Company * c2) const {
                    return c1 -> mId < c2 -> mId;
                }
        };
    };
};

#ifndef __PROGTEST__

void testCompare() {
    CVATRegister::Company::CompareNameAddr cmpNA;

    assert( cmpNA("abc", "def"));
    assert(!cmpNA("def", "abc"));
    assert(!cmpNA("abc", "abc"));

    assert( cmpNA("abc", "DEF"));
    assert( cmpNA("ABC", "def"));
    assert(!cmpNA("DEF", "abc"));
    assert(!cmpNA("def", "ABC"));
    assert(!cmpNA("ABC", "abc"));

    assert( cmpNA("abc", "abcdef"));
    assert(!cmpNA("abcdef", "abc"));


    CVATRegister::Company::CompareNameAddr cmpId;

    assert( cmpId("abc", "def"));
    assert(!cmpId("def", "abc"));
    assert(!cmpId("abc", "abc"));
}

int main ( void ) {

    testCompare();
    return 0;

    string name, addr;
    unsigned int sumIncome;

    CVATRegister b1;
    assert( b1.newCompany ( "ACME", "Thakurova", "666/666" ) );
    assert( b1.newCompany ( "ACME", "Kolejni", "666/666/666" ) );
    assert( b1.newCompany ( "Dummy", "Thakurova", "123456" ) );
    assert( b1.invoice ( "666/666", 2000 ) );
    assert( b1.medianInvoice () == 2000 );
    assert( b1.invoice ( "666/666/666", 3000 ) );
    assert( b1.medianInvoice () == 3000 );
    assert( b1.invoice ( "123456", 4000 ) );
    assert( b1.medianInvoice () == 3000 );
    assert( b1.invoice ( "aCmE", "Kolejni", 5000 ) );
    assert( b1.medianInvoice () == 4000 );
    assert( b1.audit ( "ACME", "Kolejni", sumIncome ) && sumIncome == 8000 );
    assert( b1.audit ( "123456", sumIncome ) && sumIncome == 4000 );
    assert( b1.firstCompany ( name, addr ) && name == "ACME" && addr == "Kolejni" );
    assert( b1.nextCompany ( name, addr ) && name == "ACME" && addr == "Thakurova" );
    assert( b1.nextCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
    assert(!b1.nextCompany ( name, addr ) );
    assert( b1.cancelCompany ( "ACME", "KoLeJnI" ) );
    assert( b1.medianInvoice () == 4000 );
    assert( b1.cancelCompany ( "666/666" ) );
    assert( b1.medianInvoice () == 4000 );
    assert( b1.invoice ( "123456", 100 ) );
    assert( b1.medianInvoice () == 3000 );
    assert( b1.invoice ( "123456", 300 ) );
    assert( b1.medianInvoice () == 3000 );
    assert( b1.invoice ( "123456", 200 ) );
    assert( b1.medianInvoice () == 2000 );
    assert( b1.invoice ( "123456", 230 ) );
    assert( b1.medianInvoice () == 2000 );
    assert( b1.invoice ( "123456", 830 ) );
    assert( b1.medianInvoice () == 830 );
    assert( b1.invoice ( "123456", 1830 ) );
    assert( b1.medianInvoice () == 1830 );
    assert( b1.invoice ( "123456", 2830 ) );
    assert( b1.medianInvoice () == 1830 );
    assert( b1.invoice ( "123456", 2830 ) );
    assert( b1.medianInvoice () == 2000 );
    assert( b1.invoice ( "123456", 3200 ) );
    assert( b1.medianInvoice () == 2000 );
    assert( b1.firstCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
    assert(!b1.nextCompany ( name, addr ) );
    assert(b1.cancelCompany ( "123456" ) );
    assert(!b1.firstCompany ( name, addr ) );

    CVATRegister b2;
    assert( b2.newCompany ( "ACME", "Kolejni", "abcdef" ) );
    assert( b2.newCompany ( "Dummy", "Kolejni", "123456" ) );
    assert(!b2.newCompany ( "AcMe", "kOlEjNi", "1234" ) );
    assert( b2.newCompany ( "Dummy", "Thakurova", "ABCDEF" ) );
    assert( b2.medianInvoice () == 0 );
    assert( b2.invoice ( "ABCDEF", 1000 ) );
    assert( b2.medianInvoice () == 1000 );
    assert( b2.invoice ( "abcdef", 2000 ) );
    assert( b2.medianInvoice () == 2000 );
    assert( b2.invoice ( "aCMe", "kOlEjNi", 3000 ) );
    assert( b2.medianInvoice () == 2000 );
    assert(!b2.invoice ( "1234567", 100 ) );
    assert(!b2.invoice ( "ACE", "Kolejni", 100 ) );
    assert(!b2.invoice ( "ACME", "Thakurova", 100 ) );
    assert(!b2.audit ( "1234567", sumIncome ) );
    assert(!b2.audit ( "ACE", "Kolejni", sumIncome ) );
    assert(!b2.audit ( "ACME", "Thakurova", sumIncome ) );
    assert(!b2.cancelCompany ( "1234567" ) );
    assert(!b2.cancelCompany ( "ACE", "Kolejni" ) );
    assert(!b2.cancelCompany ( "ACME", "Thakurova" ) );
    assert( b2.cancelCompany ( "abcdef" ) );
    assert( b2.medianInvoice () == 2000 );
    assert(!b2.cancelCompany ( "abcdef" ) );
    assert( b2.newCompany ( "ACME", "Kolejni", "abcdef" ) );
    assert( b2.cancelCompany ( "ACME", "Kolejni" ) );
    assert(!b2.cancelCompany ( "ACME", "Kolejni" ) );

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
