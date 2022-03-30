#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <stack>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

class CTimeStamp {
    private:
        uint16_t mYear, mMonth, mDay, mHour, mMinute, mSecond;

    public:
        explicit CTimeStamp(
                const uint16_t year,
                const uint16_t month,
                const uint16_t day,
                const uint16_t hour,
                const uint16_t minute,
                const uint16_t second)
            : mYear(year), mMonth(month), mDay(day),
            mHour(hour), mMinute(minute), mSecond(second) {}

        bool isBefore( const CTimeStamp & time) const {
            if (mYear   < time.mYear  ) return true; if (mYear   > time.mYear  ) return false; if (mMonth  < time.mMonth ) return true;
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

        void print() const {
            cout << "(" << mYear << "-" << mMonth << "-" << mDay << "-" << mHour << "-" << mMinute << "-" << mSecond << ")";
        }
};
class CContact {
    private:
        CTimeStamp mTime;
        int mNumber1, mNumber2;
    public:
        explicit CContact(const CTimeStamp time, const int number1, const int number2)
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
        const CTimeStamp & getTime() const {
            return mTime;
        }
        void print() const {
            mTime.print();
            cout << "[" << mNumber1 << " <-> " << mNumber2 << "]";
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

        void print() const {
            for (auto & c : contacts) {
                c.print();
                cout << endl;
            }
        }

        struct ContactOrder {
            size_t order;
            int number;
        };
        vector<int> listContacts(const int number) const {
            vector<ContactOrder> list;
            size_t order = 0;
            int numb;
            for (const auto & c : contacts) {
                if (c.hasNumber(number, numb)) {
                    list.push_back({order++, numb});
                }
            }
            return filterAndSort(list);
        }

        vector<int> listContacts(const int number, const CTimeStamp from, const CTimeStamp to) const {
            vector<ContactOrder> list;
            size_t order = 0;
            int numb;
            for (const auto & c : contacts) {
                if (c.hasNumber(number, numb)
                        && from.isBefore(c.getTime())
                        && c.getTime().isBefore(to)
                   ) {
                    list.push_back({order++, numb});
                }
            }
            return filterAndSort(list);
        }

        static bool compareNumbers(const ContactOrder & o1, const ContactOrder & o2) {
            return o1.number < o2.number;
        }
        static bool compareOrders(const ContactOrder & o1, const ContactOrder & o2) {
            return o1.order < o2.order;
        }
        vector<int> filterAndSort(vector<ContactOrder> & list) const {
            if (list.size() == 0) {
                vector<int> empty;
                return empty;
            }

            sort(list.begin(), list.end(), compareNumbers);

            vector<ContactOrder> unique;
            ContactOrder & latest = list.at(0);
            unique.push_back(latest);
            for (const auto & c : list) {
                if (c.number != latest.number) {
                    unique.push_back(c);
                    latest = c;
                }
            }

            vector<int> toReturn;
            sort(unique.begin(), unique.end(), compareOrders);
            for (const auto & c : unique) {
                toReturn.push_back(c.number);
            }
            return toReturn;
        }
};

#ifndef __PROGTEST__
int main ()
{
  CEFaceMask test;

  test . addContact ( CContact ( CTimeStamp ( 2021, 1, 10, 12, 40, 10 ), 111111111, 222222222 ) );
  test . addContact ( CContact ( CTimeStamp ( 2021, 1, 12, 12, 40, 10 ), 333333333, 222222222 ) )
       . addContact ( CContact ( CTimeStamp ( 2021, 2, 14, 15, 30, 28 ), 222222222, 444444444 ) );
  test . addContact ( CContact ( CTimeStamp ( 2021, 2, 15, 18, 0, 0 ), 555555555, 444444444 ) );
  assert ( test . getSuperSpreaders ( CTimeStamp ( 2021, 1, 1, 0, 0, 0 ), CTimeStamp ( 2022, 1, 1, 0, 0, 0 ) ) == (vector<int> {222222222}) );
  test . addContact ( CContact ( CTimeStamp ( 2021, 3, 20, 18, 0, 0 ), 444444444, 666666666 ) );
  test . addContact ( CContact ( CTimeStamp ( 2021, 3, 25, 0, 0, 0 ), 111111111, 666666666 ) );
  assert ( test . getSuperSpreaders ( CTimeStamp ( 2021, 1, 1, 0, 0, 0 ), CTimeStamp ( 2022, 1, 1, 0, 0, 0 ) ) == (vector<int> {222222222, 444444444}) );
  return 0;
}
#endif /* __PROGTEST__ */
