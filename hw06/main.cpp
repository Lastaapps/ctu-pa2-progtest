#ifndef __PROGTEST__
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <stdexcept>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <memory>
using namespace std;
#endif /* __PROGTEST__ */

class DataType {
    public:
        virtual ~DataType() {}
        virtual bool operator==(const DataType & type) const = 0;
        bool operator!=(const DataType & type) const {
            return !(*this == type);
        }
        virtual size_t getSize() const = 0;
        virtual void printTo(ostream & out) const = 0;
        virtual DataType * clone() const = 0;
};

class CDataTypeInt : public DataType {
    public:
        CDataTypeInt() {}
        size_t getSize() const override { return 4; }
        bool operator==(const DataType & type) const override {
            return nullptr != dynamic_cast<const CDataTypeInt*>(&type);
        }
        void printTo(ostream & out) const override {
            out << "int";
        }
        virtual DataType * clone() const override { return new CDataTypeInt(); };
};
class CDataTypeDouble : public DataType {
    public:
        CDataTypeDouble() {}
        size_t getSize() const override { return 8; }
        bool operator==(const DataType & type) const override {
            return nullptr != dynamic_cast<const CDataTypeDouble*>(&type);
        }
        void printTo(ostream & out) const override {
            out << "double";
        }
        virtual DataType * clone() const override { return new CDataTypeDouble(); };
};
class CDataTypeEnum : public DataType {
    private:
        vector<string> values;
    public:
        CDataTypeEnum() {}
        CDataTypeEnum(const CDataTypeEnum & src) : values(src.values) {}
        CDataTypeEnum & add(string str) {
            for (const string & name : values)
                if (str == name) throw invalid_argument("Duplicate enum value: " + str);
            values.emplace_back(str);
            return *this;
        }
        size_t getSize() const override { return 4; }
        bool operator==(const DataType & type) const override {
            const CDataTypeEnum * casted;
            if (nullptr == (casted = dynamic_cast<const CDataTypeEnum*>(&type)))
                return false;
            return values == casted -> values;
        }
        void printTo(ostream & out) const override {
            out << "enum {";
            bool isFirst = true;
            for (const string & str : values) {
                if (isFirst) isFirst = false; else out << ", ";
                out << str;
            }
            out << "}";
        }
        virtual DataType * clone() const override { return new CDataTypeEnum(*this); };
};
class CDataTypeStruct : public DataType {
    private:
        vector<pair<string, unique_ptr<DataType>>> data;
    public:
        CDataTypeStruct() {}
        CDataTypeStruct(const CDataTypeStruct & src) {
            for (const auto & pair : src.data)
                data.push_back(make_pair(pair.first, unique_ptr<DataType>(pair.second -> clone())));
        }
        CDataTypeStruct & addField(string str, const DataType & type) {
            for (const auto & [name, any] : data) {
                if (name == str) throw invalid_argument("Duplicate field: " + str);
            }
            data.emplace_back(make_pair(str, type.clone()));
            return *this;
        }
        const DataType & field(string str) const {
            for (const auto & [name, type] : data) {
                if (name == str) return *type;
            }
            throw invalid_argument("Unknown field: " + str);
        }
        size_t getSize() const override {
            size_t size = 0;
            for (const auto & [name, type] : data)
                size += type -> getSize();
            return size;
        }
        bool operator==(const DataType & type) const override {
            const CDataTypeStruct * casted;
            if (nullptr == (casted = dynamic_cast<const CDataTypeStruct*>(&type)))
                return false;
            const vector<pair<string, unique_ptr<DataType>>>& other = casted -> data;
            if (data.size() != other.size()) return false;
            for (size_t i = 0; i < data.size(); i++)
                if (*data[i].second != *other[i].second) return false;
            return true;
        }
        void printTo(ostream & out) const override {
            out << "struct { ";
            for (const auto & [name, type] : data) {
                type -> printTo(out);
                out << " " << name << "; ";
            }
            out << "}";
        }
        virtual DataType * clone() const override { return new CDataTypeStruct(*this); };
};

ostream & operator<<(ostream & out, const DataType & data) {
    data.printTo(out);
    return out;
}

#ifndef __PROGTEST__
static bool whitespaceMatch(const string & a, const string & b){
    cout << "A\n" << a << endl;
    cout << "B\n" << b << endl;
    cout << setw(80) << setfill('-') << "" << endl;
    auto aItr = a.begin();
    auto bItr = b.begin();
    while (true) {
        while (aItr != a.end() && iswspace(*aItr)) aItr++;
        while (bItr != b.end() && iswspace(*bItr)) bItr++;
        if (aItr == a.end() && bItr == b.end()) return true;
        if (aItr == a.end() || bItr == b.end()) return false;
        if (*aItr != *bItr) return false;
        aItr++;
        bItr++;
    }
}
template <typename T_>
static bool whitespaceMatch(const T_ & x, const string & ref) {
    ostringstream oss;
    oss << x;
    return whitespaceMatch(oss.str (), ref);
}
int main(void){

    CDataTypeStruct a = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW" ).
                add("FIXED" ).
                add("BROKEN" ).
                add("DEAD")).
        addField("m_Ratio", CDataTypeDouble ());

    CDataTypeStruct b = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW" ).
                add("FIXED" ).
                add("BROKEN" ).
                add("READY")).
        addField("m_Ratio", CDataTypeDouble ());

    CDataTypeStruct c = CDataTypeStruct () .
        addField("m_First", CDataTypeInt ()).
        addField("m_Second", CDataTypeEnum ().
                add("NEW" ).
                add("FIXED" ).
                add("BROKEN" ).
                add("DEAD")).
        addField("m_Third", CDataTypeDouble ());

    CDataTypeStruct d = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW" ).
                add("FIXED" ).
                add("BROKEN" ).
                add("DEAD")).
        addField("m_Ratio", CDataTypeInt ());
    assert( whitespaceMatch(a, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Status;\n"
                " double m_Ratio;\n"
                "}"));

    assert( whitespaceMatch(b, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " READY\n"
                " } m_Status;\n"
                " double m_Ratio;\n"
                "}"));

    assert( whitespaceMatch(c, "struct\n"
                "{\n"
                " int m_First;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Second;\n"
                " double m_Third;\n"
                "}"));

    assert( whitespaceMatch(d, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Status;\n"
                " int m_Ratio;\n"
                "}"));

    assert( a != b);
    assert( a == c);
    assert( a != d);
    assert( a.field("m_Status")== CDataTypeEnum ().add("NEW" ).add("FIXED" ).add("BROKEN" ).add("DEAD" ));
    assert( a.field("m_Status")!= CDataTypeEnum ().add("NEW" ).add("BROKEN" ).add("FIXED" ).add("DEAD" ));
    assert( a != CDataTypeInt());
    assert( whitespaceMatch(a.field("m_Status" ), "enum\n"
                "{\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                "}"));

    CDataTypeStruct aOld = a;
    b.addField("m_Other", CDataTypeDouble ());

    a.addField("m_Sum", CDataTypeInt ());

    assert( a != aOld);
    assert( a != c);
    assert( aOld == c);
    assert( whitespaceMatch(a, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Status;\n"
                " double m_Ratio;\n"
                " int m_Sum;\n"
                "}"));

    assert( whitespaceMatch(b, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " READY\n"
                " } m_Status;\n"
                " double m_Ratio;\n"
                " double m_Other;\n"
                "}"));

    c.addField("m_Another", a.field("m_Status" ));

    assert( whitespaceMatch(c, "struct\n"
                "{\n"
                " int m_First;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Second;\n"
                " double m_Third;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Another;\n"
                "}"));

    d.addField("m_Another", a.field("m_Ratio" ));

    assert( whitespaceMatch(d, "struct\n"
                "{\n"
                " int m_Length;\n"
                " enum\n"
                " {\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                " } m_Status;\n"
                " int m_Ratio;\n"
                " double m_Another;\n"
                "}"));

    assert( a.getSize () == 20);
    assert( b.getSize () == 24);
    try {
        a.addField("m_Status", CDataTypeInt ());
        assert( "addField: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what () == "Duplicate field: m_Status"sv);
    }

    try {
        cout << a.field("m_Fail")<< endl;
        assert( "field: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what () == "Unknown field: m_Fail"sv);
    }

    try {
        CDataTypeEnum en;
        en.add("FIRST").
            add("SECOND").
            add("FIRST");
        assert( "add: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what () == "Duplicate enum value: FIRST"sv);
    }

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
