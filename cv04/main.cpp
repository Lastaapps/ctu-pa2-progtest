#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <array>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

class CTimeStamp {
    private:
        uint16_t mYear, mMonth, mDay, mHour, mMinute, mSecond;

    public:
        explicit CTimeStamp(
                uint16_t year, uint16_t month, uint16_t day,
                uint16_t hour, uint16_t minute, uint16_t second)
            : mYear(year), mMonth(month), mDay(day),
            mHour(hour), mMinute(minute), mSecond(second) {}

        bool isBefore( CTimeStamp & time) const {
            if (mYear   < time.mYear  ) return true;
            if (mYear   > time.mYear  ) return false;
            if (mMonth  < time.mMonth ) return true;
            if (mMonth  > time.mMonth ) return false;
            if (mDay    < time.mDay   ) return true;
            if (mDay    > time.mDay   ) return false;
            if (mHour   < time.mHour  ) return true;
            if (mHour   > time.mHour  ) return false;
            if (mMinute < time.mMinute) return true;
            if (mMinute > time.mMinute) return false;
            if (mSecond < time.mSecond) return true;
            if (mSecond > time.mSecond) return false;
            return true; // are the same
        }
};

class CContact {
    private:
        CTimeStamp mTime;
        int mNumber1, mNumber2;
    public:
        explicit CContact(CTimeStamp time, int number1, int number2)
            : mTime(time), mNumber1(number1), mNumber2(number2) {}

        bool hasNumber(const int number, int & other) const {
            if (mNumber1 == number) {
                other = mNumber2;
                return true;
            }
            if (mNumber2 == number) {
                other = mNumber1;
                return true;
            }
            return false;
        }
        bool hasSameNubers() const {
            return mNumber1 == mNumber2;
        }
        CTimeStamp & getTime() {
            return mTime;
        }
};

class CEFaceMask {
    private:
        vector<CContact> contacts;
    public:
        CEFaceMask & addContact(CContact contact) {
            if (contact.hasSameNubers()) return *this;
            contacts.push_back(contact);
            return *this;
        }

        vector<int> listContacts(const int number) const {
            vector<int> list;
            int numb;
            for (auto & c : contacts) {
                if (c.hasNumber(number, numb)) {
                    list.push_back(numb);
                }
            }
            return list;
        }

        vector<int> listContacts(const int number, CTimeStamp from, CTimeStamp to) {
            vector<int> list;
            int numb;
            for (auto & c : contacts) {
                if (c.hasNumber(number, numb)
                        && from.isBefore(c.getTime())
                        && c.getTime().isBefore(to)
                   ) {
                    list.push_back(numb);
                }
            }
            return list;
        }
};


#ifndef __PROGTEST__
int main () {
    CEFaceMask test;

    test.addContact(CContact ( CTimeStamp ( 2021, 1, 10, 12, 40, 10 ), 123456789, 999888777 ) );
    test.addContact(CContact ( CTimeStamp ( 2021, 1, 12, 12, 40, 10 ), 123456789, 111222333 ) )
        .addContact(CContact ( CTimeStamp ( 2021, 2, 5, 15, 30, 28 ), 999888777, 555000222 ) );
    test.addContact(CContact ( CTimeStamp ( 2021, 2, 21, 18, 0, 0 ), 123456789, 999888777 ) );
    test.addContact(CContact ( CTimeStamp ( 2021, 1, 5, 18, 0, 0 ), 123456789, 456456456 ) );
    test.addContact(CContact ( CTimeStamp ( 2021, 2, 1, 0, 0, 0 ), 123456789, 123456789 ) );
    assert(test.listContacts ( 123456789 ) == (vector<int> {999888777, 111222333, 456456456}) );
    assert(test.listContacts ( 999888777 ) == (vector<int> {123456789, 555000222}) );
    assert(test.listContacts ( 191919191 ) == (vector<int> {}) );
    assert(test.listContacts ( 123456789, CTimeStamp ( 2021, 1, 5, 18, 0, 0 ), CTimeStamp ( 2021, 2, 21, 18, 0, 0 ) ) == (vector<int> {999888777, 111222333, 456456456}) );
    assert(test.listContacts ( 123456789, CTimeStamp ( 2021, 1, 5, 18, 0, 1 ), CTimeStamp ( 2021, 2, 21, 17, 59, 59 ) ) == (vector<int> {999888777, 111222333}) );
    assert(test.listContacts ( 123456789, CTimeStamp ( 2021, 1, 10, 12, 41, 9 ), CTimeStamp ( 2021, 2, 21, 17, 59, 59 ) ) == (vector<int> {111222333}) );
    return 0;
}
#endif /* __PROGTEST__ */
