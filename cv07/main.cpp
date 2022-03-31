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

        bool hasSameNubers() const {
            return mNumber1 == mNumber2;
        }
        const CTimeStamp & getTime() const {
            return mTime;
        }
        pair<int, int> toPair() const {
            return pair<int, int>(mNumber1, mNumber2);
        }
        void print() const {
            mTime.print();
            cout << "[" << mNumber1 << " <-> " << mNumber2 << "]";
        }
        friend class CEFaceMask;
};
class CEFaceMask {
    private:
        vector<CContact> contacts;
    public:
        CEFaceMask & addContact(CContact contact) {
            // filter contacts with self
            if (!contact.hasSameNubers())
                contacts.push_back(contact);
            return *this;
        }

        vector<int> getSuperSpreaders(const CTimeStamp & from, const CTimeStamp & to) const {
            // create map with occurance of each number in pairs
            set<pair<int, int>> filter;
            for (const auto & c : contacts) {
                // filter by date
                if ( from.isBefore(c.getTime()) && c.getTime().isBefore(to)) {
                    pair<int, int> data = c.toPair();
                    pair<int, int> reversed = {data.second, data.first};
                    if (filter.find(data) == filter.end() && filter.find(reversed) == filter.end()) {
                        filter.emplace(data);
                    }
                }
            }

            map<int, int> map;
            for (const auto & [c1, c2] : filter) {
                    // add to map
                    map.emplace(c1, 0).first -> second ++;
                    map.emplace(c2, 0).first -> second ++;
            }
            vector<pair<int, int>> pairs;
            vector<int> result;
            if (map.empty()) return result;

            // move map entries into a vector
            for(auto const& [key, value] : map)
                pairs.push_back({key, value});

            // sort by contacts
            sort(pairs.begin(), pairs.end(),
                    [](const pair<int, int> p1, const pair<int, int> p2){
                    return p1.second > p2.second;
                    });
            // sorted, must be the biggest occurance found
            int biggest = pairs.begin() -> second;
            for (auto const& a : pairs) {
                // when orrucance differs e.g. is lower, stop the loop
                if (a.second != biggest) break;
                result.push_back(a.first);
            }
            sort(result.begin(), result.end());
            return result;
        }

        void print() const {
            for (auto & c : contacts) {
                c.print();
                cout << endl;
            }
        }
};

#ifndef __PROGTEST__

void test1() {
    CEFaceMask test;
    CTimeStamp from = CTimeStamp ( 2021, 1, 1, 0, 0, 0 );
    CTimeStamp to = CTimeStamp ( 2022, 1, 1, 0, 0, 0 );
    CTimeStamp add = CTimeStamp ( 2021, 1, 5, 0, 0, 0 );
    test.addContact ( CContact ( CTimeStamp ( 2020, 1, 10, 12, 40, 10 ), 8, 9 ) );
    test.addContact ( CContact ( CTimeStamp ( 2022, 1, 10, 12, 40, 10 ), 8, 9 ) );
    assert( test.getSuperSpreaders ( CTimeStamp ( 2021, 1, 1, 0, 0, 0 ), CTimeStamp ( 2022, 1, 1, 0, 0, 0 ) ) == vector<int> {});
    test.addContact ( CContact ( add, 1, 2 ) );
    test.addContact ( CContact ( add, 5, 6 ) );
    test.addContact ( CContact ( add, 3, 4 ) );
    test.addContact ( CContact ( add, 7, 8 ) );
    test.addContact ( CContact ( add, 6, 7 ) );
    test.addContact ( CContact ( add, 4, 5 ) );
    test.addContact ( CContact ( add, 2, 3 ) );
    test.addContact ( CContact ( add, 8, 9 ) );
    assert( test.getSuperSpreaders (from, to) == (vector<int> { 2, 3, 4, 5, 6, 7, 8 }) );
    test.addContact ( CContact (add, 3, 5 ) );
    assert( test.getSuperSpreaders (from, to) == (vector<int> { 3, 5 }));
    test.addContact ( CContact ( add, 1, 6 ) );
    assert( test.getSuperSpreaders (from, to) == (vector<int> { 3, 5, 6 }));
    // vector<int> vec = test.getSuperSpreaders (from, to);
    // for (auto a : vec) cout << a << ", ";
    // cout << endl;
}

int main () {

    test1();

    CEFaceMask test;

    test.addContact ( CContact ( CTimeStamp ( 2021, 1, 10, 12, 40, 10 ), 111111111, 222222222 ) );
    test.addContact ( CContact ( CTimeStamp ( 2021, 1, 12, 12, 40, 10 ), 333333333, 222222222 ) )
        .addContact ( CContact ( CTimeStamp ( 2021, 2, 14, 15, 30, 28 ), 222222222, 444444444 ) );
    test.addContact ( CContact ( CTimeStamp ( 2021, 2, 15, 18, 0, 0 ), 555555555, 444444444 ) );
    assert( test.getSuperSpreaders ( CTimeStamp ( 2021, 1, 1, 0, 0, 0 ), CTimeStamp ( 2022, 1, 1, 0, 0, 0 ) ) == (vector<int> {222222222}) );
    test.addContact ( CContact ( CTimeStamp ( 2021, 3, 20, 18, 0, 0 ), 444444444, 666666666 ) );
    test.addContact ( CContact ( CTimeStamp ( 2021, 3, 25, 0, 0, 0 ), 111111111, 666666666 ) );
    assert( test.getSuperSpreaders ( CTimeStamp ( 2021, 1, 1, 0, 0, 0 ), CTimeStamp ( 2022, 1, 1, 0, 0, 0 ) ) == (vector<int> {222222222, 444444444}) );

    cout << "All tests passed!" << endl;
    return 0;
}
#endif /* __PROGTEST__ */
