#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <memory>
using namespace std;
#endif /* __PROGTEST__ */

/** Holds date, year, month and day of month */
class CDate {
    private:
        uint16_t mY;
        uint8_t mM, mD;
    public:
        /** Construct new date
         * @param y - year
         * @param m - moth
         * @param d - day of month */
        CDate(uint16_t y, uint8_t m, uint8_t d);
        /**
         * Compare two dates, earlier < later
         * @param o other date to compare to
         * @return true if the toher date is latter then this */
        bool operator<(const CDate & o) const;
        /**
         * Compare two dates, earlier > later
         * @param o other date to compare to
         * @return true if the toher date is earlier then this */
        bool operator>(const CDate & o) const;
        /** Checks if dates are the same
         * @param o other date
         * @return it dates are the same */
        bool operator==(const CDate & o) const;
        /** Checks if dates are not the same
         * @param o other date
         * @return it dates are not the same */
        inline bool operator!=(const CDate & o) const;
        /** Prints date in [yyyy-mm-dd] format
         * @param out stream to print to
         * @param date date to print
         * @return the same out stream as passed in out param */
        friend ostream & operator<<(ostream & out, const CDate & date);
};

/** Holds item name and count
 * replacement for pair<string, int> */
struct NameCount {
    string name;
    int count;
    NameCount(string n, int c): name(move(n)), count(c) {}
};
/** Holds all the item info - name, date and count */
struct StoreItem {
    string name;
    CDate date;
    int count;
    StoreItem(string n, const CDate & d, int c): name(move(n)), date(d), count(c) {}
    /** Print item in (name, date, count) format
     * @param out stream to print to
     * @param item item to print
     * @return the same out stream as passed in out param */
    friend ostream & operator<<(ostream & out, const StoreItem & item);
};

namespace cmp {
    /** Comparator using default date < operator
     * kind of useless, but makes code more coherent */
    struct DateCmp {
        bool operator()(const CDate & i1, const CDate & i2) const;
    };
    /** Compares two items according this rules
     * date1  > date2
     * name1  > name2
     * count1 < count2
     * It's ised is set sorting in expired method
     * @return true if all the rules are mached */
    struct DateComplexCmpInv {
        bool operator()(const StoreItem & i1, const StoreItem & i2) const;
    };
    /** Compares two NameCounts using these rules
     * count1 > count2
     * name1  > name2
     * so you can se, they are inverse to the normal < operator
     * @return true if the rules are matched */
    struct CountCmpInv {
        bool operator()(const NameCount & i1, const NameCount & i2) const;
    };
}

// shortcuts for CSupermarket
typedef map<CDate, int, cmp::DateCmp> DateCountMap;
typedef unordered_map<string, DateCountMap> MainMap;
typedef list<pair<string,int>> ProdList;

namespace cmp {
    /** Compares two MainMap iterators
     * uses string < operator under the hood */
    struct MainMapItrCmp {
        int operator()(const MainMap::iterator & itr1, const MainMap::iterator & itr2) const; 
    };
    /** Computes the hash code for a MainMap iterator
     * uses string hash functor under the hood */
    struct MainMapItrHash {
        int operator()(const MainMap::iterator & itr) const;
    };
}

/** Manages all the storage management */
class CSupermarket {
    // stores item in the way optimal for sell() method
    MainMap mMap;
    // holds modified keys
    unordered_multimap<string, shared_ptr<string>> mKeys;
    // stores item in the way optimal for expired() method
    set<StoreItem, cmp::DateComplexCmpInv> mSet;
    public:
    /** Creates an empty instasce of storage */
    CSupermarket();
    ~CSupermarket();
    /** Stores an item in the supermarket storage
     * @param name name of the item
     * @param expiryDate date when the item expires
     * @param count count of the item
     * @return reference to this to enable chaining */
    CSupermarket & store(string name, const CDate & expiryDate, int count);

    private:
    /** Replacement for pair<ProdList::iterator, MainMap::iterator>
     * used in sell() method implementation */
    struct AdvancedItem {
        ProdList::iterator listItr;
        MainMap::iterator mapItr;
    };

    public:
    /** Sells item from storage according to assignment.txt description
     * @param shoppingList list of items to sell
     * @return reference to this to enable chaining */
    CSupermarket & sell(ProdList & shoppingList);
    private:
    /** Tries to remove items from storage
     * @param mapItr place to take items from
     * @param count how many items at most can be taken
     * @return the number of items left/not taken from storage */
    inline int doBusiness(MainMap::iterator & mapItr, int count);
    /** Removes empty maps from MainMap
     * @param list of changed maps that can be not empty */
    inline void cleanUpMap(list<AdvancedItem> & data);
    /** Find item in MainMap with exact name match or
     * one with one letter changed (there cannot be more like this one)
     * @param name name to search for
     * @param mapOut iterator to the place found, may be changed even if
     *        the method returns false
     * @return true if an item was found, false otherwise */
    inline bool findItem(const string & name, MainMap::iterator & mapOut);
    /** Checks if two names are the same except one character
     * Not used anymore, left for nostalgia reasons
     * @param str1 first string to compare
     * @param str2 second string to compare
     * @return true if the strings match */
    inline bool nameMatch(const string & str1, const string & str2) const;

    /** Insert an item to mSet. If the item already exists, the count is added
     * @param name the name of the item
     * @param date the date of item expiration
     * @param count the amounth of the item added to the storage */
    void insertOrUpdateToSet(const string & name, const CDate & date, int count);
    /** Find item in set and replaces its count field
     * Item with same name and date must exist int the set
     * If count is 0, item is removed from the set
     * @param name the name of the item
     * @param date the expiration date of the item
     * @param count the number of the item remaining in the storage */
    void updateInSet(const string & name, const CDate & date, int count);

    /** Inserts keys parts into separate index for fast spell error correction
     * uses mKey map
     * cannot be called for already inserted item, otherwise diplicities may ocure!
     * @param name name of newly added item */
    void insertMapKeys(const string & name);
    /** Removes keys inserted earlier by insertMapKeys() method for the same name
     * called when the last item of the name was removed
     * @param name name of newly added item */
    void removeMapKeys(const string & name);
    /** Tries to match searched name against keys already stored
     * @param search name to search for
     * @param out real product name, used as optional output. May be changed
     *        even if the method returs false!
     * @return true if only one exact match occured, false otherwise */
    bool findInKeys(const string & search, string & out) const;
    /** Creates list of keys used later above.
     * the keys are the name with one character replaced by '\0'
     * for ahoj this method returns {0hoj, a0oj, ah0j, aho0}
     * @param name name to generate keys for
     * @return list of keys for the name */
    vector<string> createKeys(const string & name) const;

    public:
    /** Create a list of items that would be expired on a date given
     * @param date the date for which we want to find the expired items
     * @return list of expired items sorted by count decs */
    ProdList expired(const CDate & date) const;

    /** Prints map content, ends with '\n' char
     * @param out stream to print into */
    void printMap(ostream & out = cout) const;
    /** Prints keys content, ends with '\n' char
     * @param out stream to print into */
    void printKeys(ostream & out = cout) const;
    /** Prints set content, ends with '\n' char
     * @param out stream to print into */
    void printSet(ostream & out = cout) const;
};






CDate::CDate(uint16_t y, uint8_t m, uint8_t d) :mY(y), mM(m), mD(d) {}
bool CDate::operator<(const CDate & o) const {
    if (mY < o.mY) return true;
    if (mY > o.mY) return false;
    if (mM < o.mM) return true;
    if (mM > o.mM) return false;
    if (mD < o.mD) return true;
    if (mD > o.mD) return false;
    return false;
}
bool CDate::operator>(const CDate & o) const {
    if (mY < o.mY) return false;
    if (mY > o.mY) return true;
    if (mM < o.mM) return false;
    if (mM > o.mM) return true;
    if (mD < o.mD) return false;
    if (mD > o.mD) return true;
    return false;
}
bool CDate::operator==(const CDate & o) const {
    return mY == o.mY && mM == o.mM && mD == o.mD;
}
bool CDate::operator!=(const CDate & o) const {
    return !(*this == o);
}
ostream & operator<<(ostream & out, const CDate & date) {
    out << '[' << date.mY << '-' << (int)date.mM << '-' << (int)date.mD << ']';
    return out;
}

ostream & operator<<(ostream & out, const StoreItem & item) {
    out << '(' << item.name << ", " << item.date << ", " << item.count << ')';
    return out;
}

bool cmp::DateCmp::operator()(const CDate & i1, const CDate & i2) const {
    return i1 < i2;
}
bool cmp::DateComplexCmpInv::operator()(const StoreItem & i1, const StoreItem & i2) const {
    if (i1.date > i2.date) return true;
    if (i1.date < i2.date) return false;
    // other for equals and lower_bound
    if (i1.name > i2.name) return true;
    if (i1.name < i2.name) return false;
    if (i1.count < i2.count) return true;
    if (i1.count > i2.count) return false;
    return false;
}
bool cmp::CountCmpInv::operator()(const NameCount & i1, const NameCount & i2) const {
    if (i1.count > i2.count) return true;
    if (i1.count < i2.count) return false;
    if (i1.name > i2.name) return true;
    if (i1.name < i2.name) return false;
    return false;
}
int cmp::MainMapItrCmp::operator()(const MainMap::iterator & itr1, const MainMap::iterator & itr2) const {
    return itr1 -> first < itr2 -> first;
}
int cmp::MainMapItrHash::operator()(const MainMap::iterator & itr) const {
    return std::hash<string>()(itr -> first);
}

CSupermarket::CSupermarket() {}
CSupermarket::~CSupermarket() {}
CSupermarket & CSupermarket::store(string name, const CDate & expiryDate, int count) {
    auto mapItr = mMap.find(name);
    if (mapItr == mMap.end()) {
        DateCountMap subMap;
        subMap.insert(make_pair(expiryDate, count));
        mMap.insert(make_pair(name, subMap));
        insertMapKeys(name);
    } else {
        DateCountMap & subMap = mapItr -> second;
        auto subItr = subMap.find(expiryDate);
        if (subItr == subMap.end()) {
            subMap.insert(make_pair(expiryDate, count));
        } else {
            // same date item already exists
            subItr -> second += count;
        }
    }

    insertOrUpdateToSet(name, expiryDate, count);
    return *this;
}

CSupermarket & CSupermarket::sell(ProdList & shoppingList) {
    list<AdvancedItem> iterators;
    // find sellable items
    for (auto itr = shoppingList.begin(); itr != shoppingList.end(); itr++) {
        MainMap::iterator dataItr;
        if (findItem(itr -> first, dataItr))
            iterators.push_back({itr, dataItr});
    }
    // sell items
    for (auto & [listItr, mapItr] : iterators)
        listItr -> second = doBusiness(mapItr, listItr -> second);
    cleanUpMap(iterators);

    // drop sold items in the beginning
    while (shoppingList.size() != 0 && shoppingList.begin() -> second == 0)
        shoppingList.pop_front();

    // drop other sold items
    if (shoppingList.size() > 1) {
        auto itr = shoppingList.begin()++;
        while(itr != shoppingList.end()) {
            if (itr -> second == 0) {
                auto toRemove = itr--;
                shoppingList.erase(toRemove);
            }
            itr++;
        }
    }
    return *this;
}
inline int CSupermarket::doBusiness(MainMap::iterator & mapItr, int count) {
    const string& realName = mapItr -> first;
    DateCountMap& data = mapItr -> second;
    // empty item - date pairs
    vector<CDate> toRemove;
    for (auto &[key, value] : data) {
        if (count == 0) break;
        if (value > count) {
            value -= count;
            count = 0;
            updateInSet(realName, key, value);
            break;
        } else {
            count -= value;
            toRemove.push_back(key);
        }
    }
    for (const auto & key : toRemove) {
        data.erase(key);
        updateInSet(realName, key, 0);
    }
    return count;
}
inline void CSupermarket::cleanUpMap(list<AdvancedItem> & data) {
    unordered_set<MainMap::iterator, cmp::MainMapItrHash> iterators;
    for (auto & [listItr, mapItr] : data)
        iterators.insert(mapItr);
    for (MainMap::iterator mapItr : iterators)
        if (mapItr -> second.empty()) {
            removeMapKeys(mapItr -> first);
            mMap.erase(mapItr);
        }
}
inline bool CSupermarket::findItem(const string & name, MainMap::iterator & mapOut) {
    const auto mapItr = mMap.find(name);
    if (mapItr != mMap.end()) {
        mapOut = mapItr;
    } else {
        string realName;
        if (!findInKeys(name, realName)) return false;
        // key must exists
        mapOut = mMap.find(realName);
    }
    return true;
}
inline bool CSupermarket::nameMatch(const string & str1, const string & str2) const {
    const size_t len = str1.length();
    if (len != str2.length()) return false;
    bool diffFound = false;
    for (size_t i = 0; i < len; i++) {
        if (str1[i] != str2[i]) {
            if(diffFound) return false;
            diffFound = true;
        }
    }
    return true;
}
void CSupermarket::insertOrUpdateToSet(const string & name, const CDate & date, int count) {
    auto setItr = mSet.lower_bound(StoreItem(name, date, 0));
    if (setItr != mSet.end() && setItr -> name == name && setItr -> date == date) {
        StoreItem item = *setItr;
        item.count += count;
        mSet.erase(setItr);
        mSet.insert(item);
    } else {
        mSet.insert(StoreItem(name, date, count));
    }
}
void CSupermarket::updateInSet(const string & name, const CDate & date, int count) {
    auto setItr = mSet.lower_bound(StoreItem(name, date, 0));
    StoreItem item = *setItr;
    if (count != 0) {
        item.count = count;
        mSet.insert(item);
    }
    mSet.erase(setItr);
}
void CSupermarket::insertMapKeys(const string & name) {
    shared_ptr<string> copy = shared_ptr<string>(new string(name));
    vector<string> keys = createKeys(name);
    for (string & key : keys)
        mKeys.emplace(move(key), copy);
}
void CSupermarket::removeMapKeys(const string & name) {
    vector<string> keys = createKeys(name);
    for (const string & key : keys) {
        auto range = mKeys.equal_range(key);
        for (auto it = range.first; it != range.second; ++it) {
            if (*(it -> second) == name) {
                mKeys.erase(it);
                break;
            }
        }
    }
}
bool CSupermarket::findInKeys(const string & search, string & out) const {
    vector<string> keys = createKeys(search);
    bool anyFound = false;
    for (const string & key : keys) {
        auto range = mKeys.equal_range(key);
        auto distance = std::distance(range.first, range.second);
        if (distance == 1) {
            if (anyFound) return false; else anyFound = true;
            out = *(range.first -> second);
        } else if (distance > 1) return false;
    }
    return anyFound;
}
vector<string> CSupermarket::createKeys(const string & name) const {
    vector<string> data;
    for (size_t i = 0; i < name.length(); i++) {
        string key = string(name);
        key.at(i) = '\0';
        data.emplace_back(move(key));
    }
    return data;
}

ProdList CSupermarket::expired(const CDate & date) const {
    unordered_map<string, int> expired;
    set<NameCount, cmp::CountCmpInv> sorted;
    ProdList outList;

    auto itr = mSet.lower_bound(StoreItem("", date, 0));
    for (; itr != mSet.end(); ++itr) {
        const StoreItem & item = *itr;
        auto mapItr = expired.find(item.name);
        if (mapItr == expired.end())
            expired.insert(make_pair(item.name, item.count));
        else
            mapItr -> second += item.count;
    }
    // sort selected items by count
    for (auto const & [name, count] : expired)
        sorted.insert(NameCount(name, count));
    for (const NameCount & item : sorted)
        outList.emplace_back(make_pair(item.name, item.count));
    return outList;
}
void CSupermarket::printMap(ostream & out) const {
    out << "Map" << endl;
    for (const auto & [name, data] : mMap) {
        out << "+ " << name << endl;
        for (const auto & [date, count] : data) {
            out << "> " << date << " - " << count << endl;
        }
    }
}
void CSupermarket::printKeys(ostream & out) const {
    out << "Keys" << endl;
    for (auto itr = mKeys.begin(); itr != mKeys.end(); ) {
        const auto curr = itr;
        out << "* " << itr -> first << " - ";
        while (itr != mKeys.end() && itr->first == curr->first) {
            if (itr != curr) out << ", ";
            out  << *(itr -> second);
            ++itr;
        }
        out << endl;
    }
}
void CSupermarket::printSet(ostream & out) const {
    out << "Set" << endl;
    for (const StoreItem & item : mSet) {
        out << "/ " << item << endl;
    }
}


#ifndef __PROGTEST__
void printLine(const string msg) {
    cout << "--- " << setw(80 - 4) << setfill('-') << left << (msg + ' ') << endl;
}
void printList(const ProdList & list) {
    cout << "- ";
    for (auto const & item : list) {
        cout << item.first << " " << item.second << " -> ";
    }
    cout << "null" << endl;
}

void myTest() {
    CSupermarket s;
    ProdList l;
    CDate date = CDate(2022, 4, 1);

    l = s.expired(date);
    assert(l.size() == 0);
    s.store("nevim", CDate(2022, 3, 1), 69);
    l = s.expired(date);
    assert(l.size() == 1);
    s.store("nevim", CDate(2022, 5, 1), 69);
    l = s.expired(date);
    assert(l.size() == 1);
    s.store("nevim", CDate(2022, 4, 1), 69);
    l = s.expired(date);
    assert(l.size() == 1);

    s = CSupermarket();
    s.store("Never", date, 6);
    s.store("gonna", date, 6);
    s.store("you", date, 5);
    s.store("and", date, 2);

    s.store("a", date, 1);
    s.store("around", date, 1);
    s.store("cry", date, 1);
    s.store("desert", date, 1);
    s.store("down", date, 1);
    s.store("give", date, 1);
    s.store("goodbye", date, 1);
    s.store("hurt", date, 1);
    s.store("let", date, 1);
    s.store("lie", date, 1);
    s.store("make", date, 1);
    s.store("run", date, 1);
    s.store("say", date, 1);
    s.store("tell", date, 1);
    s.store("up", date, 1);

    l = {{"Never", 1}, {"gonna", 1}, {"give", 1}, {"you", 1}, {"up", 1}};
    s.sell(l);
    assert(l.empty());
    l = {{"neveR", 1}, {"GOnna", 1}, {"lET", 1}, {"she", 1}, {"dOVn", 1}};
    s.sell(l);
    assert((l == ProdList{{"neveR", 1}, {"GOnna", 1}, {"lET", 1}, {"she", 1}, {"dOVn", 1}}));
    l = {{"never", 1}, {"Gonna", 1}, {"lEt", 1}, {"yRu", 1}, {"doVn", 1}};
    s.sell(l);
    assert(l.empty());
    l = {{"never", 1}, {"Gonna", 1}, {"lEt", 1}, {"yRu", 1}, {"doVn", 1}};
    s.sell(l);
    assert((l == ProdList{{"lEt", 1}, {"doVn", 1}}));
    l = {{"Never", 1}, {"gonna", 1}, {"run", 1}, {"around", 1}, {"and", 1}, {"desert", 1}, {"you", 1}};
    s.sell(l);
    assert(l.empty());
    l = {{"Never", 1}, {"gonna", 1}, {"make", 1}, {"you", 1}, {"cry", 1}};
    s.sell(l);
    assert((l == ProdList{}));
    l = {{"Never", 1}, {"gonna", 1}, {"say", 1}, {"goodbye", 1}};
    s.sell(l);
    assert((l == ProdList{}));
    l = {{"Never", 1}, {"gonna", 1}, {"tell", 1}, {"a", 1}, {"lie", 1}, {"and", 1}, {"hurt", 1}, {"you", 1}};
    s.sell(l);
    assert((l == ProdList{{"Never", 1}, {"gonna", 1}, {"you", 1}}));
    l = {{"Never", 1}, {"gonna", 1}, {"tell", 1}, {"a", 1}, {"lie", 1}, {"and", 1}, {"hurt", 1}, {"you", 1}};
    s.sell(l);
    assert((l == ProdList{{"Never", 1}, {"gonna", 1}, {"tell", 1}, {"a", 1}, {"lie", 1}, {"and", 1}, {"hurt", 1}, {"you", 1}}));
}

int main(void) {
    myTest();

    CSupermarket s;

    printLine("Init");
    s.store("bread", CDate(2016, 4, 30 ), 100 )
        .store("butter", CDate(2016, 5, 10 ), 10 )
        .store("beer", CDate(2016, 8, 10 ), 50 )
        .store("bread", CDate(2016, 4, 25 ), 100 )
        .store("okey", CDate(2016, 7, 18 ), 5 );
    s.printMap();
    s.printSet();

    list<pair<string,int>> l0 = s.expired(CDate(2018, 4, 30));
    assert( l0.size () == 4 );
    assert((l0 == list<pair<string,int>> { { "bread", 200 }, { "beer", 50 }, { "butter", 10 }, { "okey", 5 } }));

    list<pair<string,int>> l1 { { "bread", 2 }, { "Coke", 5 }, { "butter", 20 } };
    printLine("Sell 01");
    printList(l1);
    s.sell(l1 );
    printList(l1);
    s.printMap();
    s.printSet();
    assert( l1.size () == 2 );
    assert((l1 == list<pair<string,int>> { { "Coke", 5 }, { "butter", 10 } }));

    list<pair<string,int>> l2 = s.expired(CDate(2016, 4, 30));
    printList(l2);
    assert( l2.size () == 1 );
    assert((l2 == list<pair<string,int>> { { "bread", 98 } }));

    list<pair<string,int>> l3 = s.expired(CDate(2016, 5, 20));
    printList(l3);
    assert((l2 == list<pair<string,int>> { { "bread", 98 } }));
    assert( l3.size () == 1 );
    assert((l3 == list<pair<string,int>> { { "bread", 198 } }));

    list<pair<string,int>> l4 { { "bread", 105 } };
    printLine("Sell 04");
    printList(l4);
    s.sell(l4 );
    printList(l4);
    s.printMap();
    s.printSet();
    assert( l4.size () == 0 );
    assert((l4 == list<pair<string,int>> { }));

    list<pair<string,int>> l5 = s.expired(CDate(2017, 1, 1));
    assert( l5.size () == 3 );
    assert((l5 == list<pair<string,int>> { { "bread", 93 }, { "beer", 50 }, { "okey", 5 } }));

    s.store("Coke", CDate(2016, 12, 31 ), 10 );
    printLine("Stored Coke");
    s.printMap();
    s.printSet();

    list<pair<string,int>> l6 { { "Cake", 1 }, { "Coke", 1 }, { "cake", 1 }, { "coke", 1 }, { "cuke", 1 }, { "Cokes", 1 } };
    printLine("Sell 06");
    printList(l6);
    s.sell(l6 );
    printList(l6);
    s.printMap();
    s.printSet();
    assert( l6.size () == 3 );
    assert((l6 == list<pair<string,int>> { { "cake", 1 }, { "cuke", 1 }, { "Cokes", 1 } }));

    list<pair<string,int>> l7 = s.expired(CDate(2017, 1, 1));
    printList(l7);
    assert( l7.size () == 4 );
    assert((l7 == list<pair<string,int>> { { "bread", 93 }, { "beer", 50 }, { "Coke", 7 }, { "okey", 5 } }));

    s.store("cake", CDate(2016, 11, 1 ), 5 );
    printLine("Stored cake");

    list<pair<string,int>> l8 { { "Cake", 1 }, { "Coke", 1 }, { "cake", 1 }, { "coke", 1 }, { "cuke", 1 } };
    printLine("Sell 08");
    printList(l8);
    s.sell(l8 );
    printList(l8);
    s.printMap();
    s.printSet();
    assert( l8.size () == 2 );
    assert((l8 == list<pair<string,int>> { { "Cake", 1 }, { "coke", 1 } }));

    list<pair<string,int>> l9 = s.expired(CDate(2017, 1, 1));
    printList(l9);
    assert( l9.size () == 5 );
    assert((l9 == list<pair<string,int>> { { "bread", 93 }, { "beer", 50 }, { "Coke", 6 }, { "okey", 5 }, { "cake", 3 } }));

    list<pair<string,int>> l10 { { "cake", 15 }, { "Cake", 2 } };
    printLine("Sell 10");
    printList(l10);
    s.sell(l10 );
    printList(l10);
    s.printMap();
    s.printSet();
    assert( l10.size () == 2 );
    assert((l10 == list<pair<string,int>> { { "cake", 12 }, { "Cake", 2 } }));

    list<pair<string,int>> l11 = s.expired(CDate(2017, 1, 1));
    printList(l11);
    assert( l11.size () == 4 );
    assert((l11 == list<pair<string,int>> { { "bread", 93 }, { "beer", 50 }, { "Coke", 6 }, { "okey", 5 } }));

    list<pair<string,int>> l12 { { "Cake", 4 } };
    printLine("Sell 12");
    printList(l12);
    s.sell(l12 );
    printList(l12);
    s.printMap();
    s.printSet();
    assert( l12.size () == 0 );
    assert((l12 == list<pair<string,int>> { }));

    list<pair<string,int>> l13 = s.expired(CDate(2017, 1, 1));
    printList(l13);
    assert( l13.size () == 4 );
    assert((l13 == list<pair<string,int>> { { "bread", 93 }, { "beer", 50 }, { "okey", 5 }, { "Coke", 2 } }));

    list<pair<string,int>> l14 { { "Beer", 20 }, { "Coke", 1 }, { "bear", 25 }, { "beer", 10 } };
    printLine("Sell 14");
    printList(l14);
    s.sell(l14 );
    printList(l14);
    s.printMap();
    s.printSet();
    assert( l14.size () == 1 );
    assert((l14 == list<pair<string,int>> { { "beer", 5 } }));

    s.store("ccccb", CDate(2019, 3, 11 ), 100 )
        .store("ccccd", CDate(2019, 6, 9 ), 100 )
        .store("dcccc", CDate(2019, 2, 14 ), 100 );
    printLine("Stored 2x ccccb, dcccc");
    s.printMap();
    s.printKeys();
    s.printSet();

    list<pair<string,int>> l15 { { "ccccc", 10 } };
    printLine("Sell 15");
    printList(l15);
    s.sell(l15 );
    printList(l15);
    s.printMap();
    s.printSet();
    assert( l15.size () == 1 );
    assert((l15 == list<pair<string,int>> { { "ccccc", 10 } }));

    cout << endl;
    printLine("It's done!");
    printLine("Yes, Mr. Frodo, it's over now.");

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
