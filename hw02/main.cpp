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
                string mName;
                string mAddr;
                string mId;
                unsigned int mAmount;
            public:
                Company(const string & name, const string & addr,
                        const string & id, unsigned int amount = 0)
                    : mName(name), mAddr(addr), mId(id), mAmount(amount) {}
                /*Company(const Company & c)
                  :mName(c.mName), mAddr(c.mAddr), mId(c.mId), mAmount(c.mAmount) {}

                  inline Company operator = (const Company & c) {
                  return Company(c);
                  }*/

                void print(ostream & out) const;

                struct CompareNameAddr {
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
                        inline bool operator() (const Company * c1, const Company * c2) const {
                            if ((*this)(c1 -> mName, c2 -> mName)) return true;
                            if ((*this)(c1 -> mAddr, c2 -> mAddr)) return true;
                            return false;
                        }
                };
                struct CompareId {
                    public:
                        inline bool operator() (const Company * c1, const Company * c2) const {
                            return c1 -> mId < c2 -> mId;
                        }
                };
        };

        void printList(ostream & out) const;
        void printIds(ostream & out) const;

    private:
        vector<const Company*> mList;
        vector<const Company*> mIds;

        bool bSearchName(const Company * c, size_t & index) const;
        bool bSearchId(const Company * c, size_t & index) const;

};

typedef CVATRegister Reg;

Reg::CVATRegister(void) {}
Reg::~CVATRegister(void) {
    for (auto ptr : mList) delete ptr;
}

bool Reg::newCompany ( const string & name, const string & addr, const string & taxID ) {
    const Company * c = new Company(name, addr, taxID);
    size_t indexName, indexId;
    if (bSearchName(c, indexName) || bSearchId(c, indexId)) {
        delete c;
        return false;
    } else {
        mList.insert(mList.begin(), c);
        mIds.insert(mIds.begin(), c);

        return true;
    }
}

bool Reg::bSearchName(const Reg::Company * c, size_t & index) const {
    Reg::Company::CompareNameAddr cmp;
    auto & list = mList;
    auto lower = lower_bound(list.begin(), list.end(), c, cmp);
    index = distance(list.begin(), lower);
    if (lower == list.end()) return false;
    return cmp(*lower, c) && cmp(c, *lower);
}

bool Reg::bSearchId(const Reg::Company * c, size_t & index) const {
    Reg::Company::CompareId cmp;
    auto & list = mIds;
    auto lower = lower_bound(list.begin(), list.end(), c, cmp);
    index = distance(list.begin(), lower);
    if (lower == list.end()) return false;
    return cmp(*lower, c) && cmp(c, *lower);
}





void Reg::Company::print(ostream & out = cout) const {
    out << "[" << mName << ", " << mAddr << ", " << mId << ", " << mAmount << "]";
}
void Reg::printList(ostream & out = cout) const {
    out << "List: Total of " << mList.size() << " items" << endl;
    for (size_t i = 0; i < mList.size(); i++) {
        out << i << ". ";
        mList[i] -> print(out);
        out << "\n";
    }
    out.flush();
}
void Reg::printIds(ostream & out = cout) const {
    out << "IDs: Total of " << mIds.size() << " items" << endl;
    for (size_t i = 0; i < mIds.size(); i++) {
        out << i << ". ";
        mIds[i] -> print(out);
        out << "\n";
    }
    out.flush();
}

#ifndef __PROGTEST__

void testCompare() {
    Reg::Company::CompareNameAddr cmpNA;

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
}

int main ( void ) {

    testCompare();

    string name, addr;
    //unsigned int sumIncome;

    CVATRegister b1;
    assert( b1.newCompany ( "ACME", "Thakurova", "666/666" ) );
    b1.printList();
    b1.printIds();
    assert( b1.newCompany ( "ACME", "Kolejni", "666/666/666" ) );
    b1.printList();
    b1.printIds();
    assert( b1.newCompany ( "Dummy", "Thakurova", "123456" ) );

    b1.printList();
    b1.printIds();
    return 0;
#ifdef something_random
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
#endif
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
