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
        operator string() const {
            string builder;
            toString(builder);
            return builder;
        }
        virtual const DataType& field(string str) const {
            throw invalid_argument("Cannot use field() for type: "s + static_cast<string>(*this));
        };
        virtual const DataType& element() const {
            throw invalid_argument("Cannot use element() for type: "s + static_cast<string>(*this));
        };
        virtual size_t getSize() const = 0;
        virtual void toString(string & builder) const = 0;
        virtual DataType * clone() const = 0;
};

class CDataTypeInt : public DataType {
    public:
        CDataTypeInt() {}
        size_t getSize() const override { return 4; }
        bool operator==(const DataType & type) const override {
            return nullptr != dynamic_cast<const CDataTypeInt*>(&type);
        }
        void toString(string & builder) const override {
            builder = "int " + builder;
        };
        virtual DataType * clone() const override { return new CDataTypeInt(); };
};
class CDataTypeDouble : public DataType {
    public:
        CDataTypeDouble() {}
        size_t getSize() const override { return 8; }
        bool operator==(const DataType & type) const override {
            return nullptr != dynamic_cast<const CDataTypeDouble*>(&type);
        }
        void toString(string & builder) const override {
            builder = "double " + builder;
        };
        virtual DataType * clone() const override { return new CDataTypeDouble(); };
};
class CDataTypeEnum : public DataType {
    private:
        vector<string> values;
    public:
        CDataTypeEnum() {}
        CDataTypeEnum(const CDataTypeEnum & src) : values(src.values) {}
        CDataTypeEnum & add(string str) {
            for(const string & name : values)
                if(str == name) throw invalid_argument("Duplicate enum value: " + str);
            values.emplace_back(str);
            return *this;
        }
        bool operator==(const DataType & type) const override {
            const CDataTypeEnum * casted;
            if(nullptr ==(casted = dynamic_cast<const CDataTypeEnum*>(&type)))
                return false;
            return values == casted -> values;
        }
        size_t getSize() const override { return 4; }
        void toString(string & builder) const override {
            string buff = "enum { ";
            bool isFirst = true;
            for(const string & str : values) {
                if(isFirst) isFirst = false; else buff += ", ";
                buff += str;
            }
            buff += " }";
            builder = buff + builder;
        };
        virtual DataType * clone() const override { return new CDataTypeEnum(*this); };
};
class CDataTypeArray : public DataType {
    private:
        size_t size;
        unique_ptr<DataType> type;
    public:
        CDataTypeArray(size_t s, const DataType & t)
            : size(s), type(unique_ptr<DataType>(t.clone())) {}
        CDataTypeArray(const CDataTypeArray & src)
            : size(src.size), type(unique_ptr<DataType>(src.type -> clone())) {}
        size_t getSize() const override {
            return size * type -> getSize();
        }
        const DataType& element() const override { return *type; }
        bool operator==(const DataType & t) const override {
            const CDataTypeArray * casted;
            if (nullptr == (casted = dynamic_cast<const CDataTypeArray*>(&t)))
                return false;
            if (size != casted -> size) return false;
            if (*type != *(casted -> type)) return false;
            return true;
        }
        void toString(string & builder) const override {
            builder += "[" + to_string(size) + "]";
            type -> toString(builder);
        }
        virtual DataType * clone() const override { return new CDataTypeArray(*this); };
        friend class CDataTypePtr;
};
class CDataTypePtr : public DataType {
    private:
        unique_ptr<DataType> type;
    public:
        CDataTypePtr(const DataType & t)
            : type(unique_ptr<DataType>(t.clone())) {}
        CDataTypePtr(const CDataTypePtr & src)
            : type(unique_ptr<DataType>(src.type -> clone())) {}
        bool operator==(const DataType & t) const override {
            const CDataTypePtr * casted;
            if (nullptr == (casted = dynamic_cast<const CDataTypePtr*>(&t)))
                return false;
            return *type == *(casted -> type);
        }
        size_t getSize() const override { return 8; }
        const DataType& element() const override { return *type; }
        void toString(string & builder) const override {
            const CDataTypeArray * arr;
            if (nullptr != (arr = dynamic_cast<const CDataTypeArray*>(&*type))) {
                builder = "(*" + builder + ")[" + to_string(arr -> size) + "]";
                type -> element().toString(builder);
            } else {
                builder = "*" + builder;
                type -> toString(builder);
            }
        }
        virtual DataType * clone() const override { return new CDataTypePtr(*this); };
};
class CDataTypeStruct : public DataType {
    private:
        vector<pair<string, unique_ptr<DataType>>> data;
    public:
        CDataTypeStruct() {}
        CDataTypeStruct(const CDataTypeStruct & src) {
            for(const auto & pair : src.data)
                data.push_back(make_pair(pair.first, unique_ptr<DataType>(pair.second -> clone())));
        }
        CDataTypeStruct & addField(string str, const DataType & type) {
            for(const auto & [name, any] : data) {
                if(name == str) throw invalid_argument("Duplicate field: " + str);
            }
            data.emplace_back(make_pair(str, type.clone()));
            return *this;
        }
        bool operator==(const DataType & type) const override {
            const CDataTypeStruct * casted;
            if(nullptr ==(casted = dynamic_cast<const CDataTypeStruct*>(&type)))
                return false;
            const vector<pair<string, unique_ptr<DataType>>>& other = casted -> data;
            if(data.size() != other.size()) return false;
            for(size_t i = 0; i < data.size(); i++)
                if(*data[i].second != *other[i].second) return false;
            return true;
        }
        const DataType & field(string str) const override {
            for(const auto & [name, type] : data) {
                if(name == str) return *type;
            }
            throw invalid_argument("Unknown field: " + str);
        }
        size_t getSize() const override {
            size_t size = 0;
            for(const auto & [name, type] : data)
                size += type -> getSize();
            return size;
        }
        void toString(string & builder) const override {
            string buff = "struct {\n";
            for(const auto & [name, type] : data) {
                const CDataTypeArray * arr;
                const CDataTypePtr * ptr;
                if (nullptr != (arr = dynamic_cast<const CDataTypeArray*>(&*type))) {
                    string tmp = name;
                    type -> toString(tmp);
                    buff += tmp + ";\n";
                } else if (nullptr != (ptr = dynamic_cast<const CDataTypePtr*>(&*type))) {
                    string tmp = name;
                    type -> toString(tmp);
                    buff += tmp + ";\n";
                } else {
                    string tmp;
                    type -> toString(tmp);
                    buff += tmp + " " + name + ";\n";
                }
            }
            buff += "}";
            builder = buff + builder;
        }
        virtual DataType * clone() const override { return new CDataTypeStruct(*this); };
};

ostream & operator<<(ostream & out, const DataType & data) {
    out << static_cast<string>(data);
    return out;
}

#ifndef __PROGTEST__
static bool whitespaceMatch(const string & a, const string & b){
    cout << "A\n" << a << endl;
    cout << "B\n" << b << endl;
    cout << setw(80) << setfill('-') << "" << endl;
    auto aItr = a.begin();
    auto bItr = b.begin();
    while(true) {
        while(aItr != a.end() && iswspace(*aItr)) aItr++;
        while(bItr != b.end() && iswspace(*bItr)) bItr++;
        if(aItr == a.end() && bItr == b.end()) return true;
        if(aItr == a.end() || bItr == b.end()) return false;
        if(*aItr != *bItr) return false;
        aItr++;
        bItr++;
    }
}
template <typename T_>
static bool whitespaceMatch(const T_ & x, const string & ref) {
    ostringstream oss;
    oss << x;
    return whitespaceMatch(oss.str(), ref);
}
void testsBasic() {
    CDataTypeStruct a = CDataTypeStruct() .
        addField("m_Length", CDataTypeInt()).
        addField("m_Status", CDataTypeEnum().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Ratio", CDataTypeDouble());

    CDataTypeStruct b = CDataTypeStruct() .
        addField("m_Length", CDataTypeInt()).
        addField("m_Status", CDataTypeEnum().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("READY")).
        addField("m_Ratio", CDataTypeDouble());

    CDataTypeStruct c = CDataTypeStruct() .
        addField("m_First", CDataTypeInt()).
        addField("m_Second", CDataTypeEnum().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Third", CDataTypeDouble());

    CDataTypeStruct d = CDataTypeStruct() .
        addField("m_Length", CDataTypeInt()).
        addField("m_Status", CDataTypeEnum().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Ratio", CDataTypeInt());
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
    assert( a.field("m_Status")== CDataTypeEnum().add("NEW").add("FIXED").add("BROKEN").add("DEAD"));
    assert( a.field("m_Status")!= CDataTypeEnum().add("NEW").add("BROKEN").add("FIXED").add("DEAD"));
    assert( a != CDataTypeInt());
    assert( whitespaceMatch(a.field("m_Status"), "enum\n"
                "{\n"
                " NEW,\n"
                " FIXED,\n"
                " BROKEN,\n"
                " DEAD\n"
                "}"));

    CDataTypeStruct aOld = a;
    b.addField("m_Other", CDataTypeDouble());

    a.addField("m_Sum", CDataTypeInt());

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

    c.addField("m_Another", a.field("m_Status"));

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

    d.addField("m_Another", a.field("m_Ratio"));

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

    assert( a.getSize() == 20);
    assert( b.getSize() == 24);
    try {
        a.addField("m_Status", CDataTypeInt());
        assert( "addField: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what() == "Duplicate field: m_Status"sv);
    }

    try {
        cout << a.field("m_Fail")<< endl;
        assert( "field: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what() == "Unknown field: m_Fail"sv);
    }

    try {
        CDataTypeEnum en;
        en.add("FIRST").
            add("SECOND").
            add("FIRST");
        assert( "add: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( e.what() == "Duplicate enum value: FIRST"sv);
    }
}

void testsAdvanced() {
    CDataTypeStruct  a = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Ratio", CDataTypeDouble ());

    CDataTypeStruct b = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("READY")).
        addField("m_Ratio", CDataTypeDouble ());

    CDataTypeStruct c =  CDataTypeStruct () .
        addField("m_First", CDataTypeInt ()).
        addField("m_Second", CDataTypeEnum ().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Third", CDataTypeDouble ());

    CDataTypeStruct  d = CDataTypeStruct () .
        addField("m_Length", CDataTypeInt ()).
        addField("m_Status", CDataTypeEnum ().
                add("NEW").
                add("FIXED").
                add("BROKEN").
                add("DEAD")).
        addField("m_Ratio", CDataTypeInt ());
    assert( whitespaceMatch(a, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "}"));

    assert( whitespaceMatch(b, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    READY\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "}"));

    assert( whitespaceMatch(c, "struct\n"
                "{\n"
                "  int m_First;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Second;\n"
                "  double m_Third;\n"
                "}"));

    assert( whitespaceMatch(d, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  int m_Ratio;\n"
                "}"));

    assert( a != b);
    assert( a == c);
    assert( a != d);
    assert( a != CDataTypeInt());
    assert( whitespaceMatch(a.field("m_Status"), "enum\n"
                "{\n"
                "  NEW,\n"
                "  FIXED,\n"
                "  BROKEN,\n"
                "  DEAD\n"
                "}"));

    b.addField("m_Other", a);

    a.addField("m_Sum", CDataTypeInt ());

    assert( whitespaceMatch(a, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "  int m_Sum;\n"
                "}"));

    assert( whitespaceMatch(b, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    READY\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      DEAD\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "  } m_Other;\n"
                "}"));

    assert( whitespaceMatch(b.field("m_Other"), "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "}"));

    assert( whitespaceMatch(b.field("m_Other").field("m_Status"), "enum\n"
                "{\n"
                "  NEW,\n"
                "  FIXED,\n"
                "  BROKEN,\n"
                "  DEAD\n"
                "}"));

    assert( a.getSize () == 20);
    assert( b.getSize () == 32);
    b.addField("m_Other1", b);
    b.addField("m_Other2", b);
    b.addField("m_Other3", b);
    assert( b.field("m_Other3").field("m_Other2").field("m_Other1")== b.field("m_Other1"));

    assert( b.getSize () == 256);

    assert( whitespaceMatch(b, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    READY\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      DEAD\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "  } m_Other;\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      READY\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        DEAD\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "    } m_Other;\n"
                "  } m_Other1;\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      READY\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        DEAD\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "    } m_Other;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        READY\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "      struct\n"
                "      {\n"
                "        int m_Length;\n"
                "        enum\n"
                "        {\n"
                "          NEW,\n"
                "          FIXED,\n"
                "          BROKEN,\n"
                "          DEAD\n"
                "        } m_Status;\n"
                "        double m_Ratio;\n"
                "      } m_Other;\n"
                "    } m_Other1;\n"
                "  } m_Other2;\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      READY\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        DEAD\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "    } m_Other;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        READY\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "      struct\n"
                "      {\n"
                "        int m_Length;\n"
                "        enum\n"
                "        {\n"
                "          NEW,\n"
                "          FIXED,\n"
                "          BROKEN,\n"
                "          DEAD\n"
                "        } m_Status;\n"
                "        double m_Ratio;\n"
                "      } m_Other;\n"
                "    } m_Other1;\n"
                "    struct\n"
                "    {\n"
                "      int m_Length;\n"
                "      enum\n"
                "      {\n"
                "        NEW,\n"
                "        FIXED,\n"
                "        BROKEN,\n"
                "        READY\n"
                "      } m_Status;\n"
                "      double m_Ratio;\n"
                "      struct\n"
                "      {\n"
                "        int m_Length;\n"
                "        enum\n"
                "        {\n"
                "          NEW,\n"
                "          FIXED,\n"
                "          BROKEN,\n"
                "          DEAD\n"
                "        } m_Status;\n"
                "        double m_Ratio;\n"
                "      } m_Other;\n"
                "      struct\n"
                "      {\n"
                "        int m_Length;\n"
                "        enum\n"
                "        {\n"
                "          NEW,\n"
                "          FIXED,\n"
                "          BROKEN,\n"
                "          READY\n"
                "        } m_Status;\n"
                "        double m_Ratio;\n"
                "        struct\n"
                "        {\n"
                "          int m_Length;\n"
                "          enum\n"
                "          {\n"
                "            NEW,\n"
                "            FIXED,\n"
                "            BROKEN,\n"
                "            DEAD\n"
                "          } m_Status;\n"
                "          double m_Ratio;\n"
                "        } m_Other;\n"
                "      } m_Other1;\n"
                "    } m_Other2;\n"
                "  } m_Other3;\n"
                "}"));

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
        cout << a.field("m_Length").field("m_Foo")<< endl;
        assert( "field: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( whitespaceMatch(e.what (), "Cannot use field() for type: int"));
    }

    try {
        cout << a.field("m_Status").field("m_Foo")<< endl;
        assert( "field: missing exception!" == nullptr);
    } catch(const invalid_argument & e){
        assert( whitespaceMatch(e.what (), "Cannot use field() for type: enum\n"
                    "{\n"
                    "  NEW,\n"
                    "  FIXED,\n"
                    "  BROKEN,\n"
                    "  DEAD\n"
                    "}"));
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

    CDataTypeArray ar1( 10, CDataTypeInt());
    assert( whitespaceMatch( ar1, "int[10]"));
    assert( whitespaceMatch( ar1.element(), "int"));
    CDataTypeArray ar2( 11, ar1);
    assert( whitespaceMatch( ar2, "int[11][10]"));
    assert( whitespaceMatch( ar2.element(), "int[10]"));
    assert( whitespaceMatch( ar2.element().element(), "int"));
    CDataTypeArray ar3( 10, CDataTypeArray( 20, CDataTypePtr( CDataTypeInt())));
    assert( whitespaceMatch( ar3, "int*[10][20]"));
    assert( whitespaceMatch( ar3.element(), "int*[20]"));
    assert( whitespaceMatch( ar3.element().element(), "int*"));
    assert( whitespaceMatch( ar3.element().element().element(), "int"));
    CDataTypePtr ar4(ar1.element());
    assert( whitespaceMatch( ar4, "int*"));
    assert( whitespaceMatch( ar4.element(), "int"));
    CDataTypePtr ar5(b.field( "m_Status"));
    assert( whitespaceMatch( ar5, "enum\n"
                "{\n"
                "  NEW,\n"
                "  FIXED,\n"
                "  BROKEN,\n"
                "  READY\n"
                "}*"));
    assert( whitespaceMatch( ar5.element(), "enum\n"
                "{\n"
                "  NEW,\n"
                "  FIXED,\n"
                "  BROKEN,\n"
                "  READY\n"
                "}"));
    CDataTypePtr ar6( ar3.element().element());
    assert( whitespaceMatch( ar6, "int**"));
    assert( whitespaceMatch( ar6.element(), "int*"));
    assert( whitespaceMatch( ar6.element().element(), "int"));
    CDataTypePtr ar7( CDataTypeArray( 50, ar6));
    assert( whitespaceMatch( ar7, "int**(*)[50]"));
    assert( whitespaceMatch( ar7.element(), "int**[50]"));
    assert( whitespaceMatch( ar7.element().element(), "int**"));
    assert( whitespaceMatch( ar7.element().element().element(), "int*"));
    assert( whitespaceMatch( ar7.element().element().element().element(), "int"));
    CDataTypeArray ar8( 25, ar7);
    assert( whitespaceMatch( ar8, "int**(*[25])[50]"));
    CDataTypePtr ar9( ar8);
    assert( whitespaceMatch( ar9, "int**(*(*)[25])[50]"));
    a.addField( "m_Ar1", ar1).
        addField( "m_Ar2", ar2).
        addField( "m_Ar3", ar3).
        addField( "m_Ar4", ar4).
        addField( "m_Ar5", ar5).
        addField( "m_Ar6", ar6).
        addField( "m_Ar7", ar7).
        addField( "m_Ar8", ar8).
        addField( "m_Ar9", ar9);
    assert( whitespaceMatch( a, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "  int m_Sum;\n"
                "  int m_Ar1[10];\n"
                "  int m_Ar2[11][10];\n"
                "  int* m_Ar3[10][20];\n"
                "  int* m_Ar4;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    READY\n"
                "  }* m_Ar5;\n"
                "  int** m_Ar6;\n"
                "  int**(* m_Ar7)[50];\n"
                "  int**(* m_Ar8[25])[50];\n"
                "  int**(*(* m_Ar9)[25])[50];\n"
                "}"));
    a.addField( "m_Madness", CDataTypeArray( 40, CDataTypePtr( a)));
    assert( whitespaceMatch( a, "struct\n"
                "{\n"
                "  int m_Length;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    DEAD\n"
                "  } m_Status;\n"
                "  double m_Ratio;\n"
                "  int m_Sum;\n"
                "  int m_Ar1[10];\n"
                "  int m_Ar2[11][10];\n"
                "  int* m_Ar3[10][20];\n"
                "  int* m_Ar4;\n"
                "  enum\n"
                "  {\n"
                "    NEW,\n"
                "    FIXED,\n"
                "    BROKEN,\n"
                "    READY\n"
                "  }* m_Ar5;\n"
                "  int** m_Ar6;\n"
                "  int**(* m_Ar7)[50];\n"
                "  int**(* m_Ar8[25])[50];\n"
                "  int**(*(* m_Ar9)[25])[50];\n"
                "  struct\n"
                "  {\n"
                "    int m_Length;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      DEAD\n"
                "    } m_Status;\n"
                "    double m_Ratio;\n"
                "    int m_Sum;\n"
                "    int m_Ar1[10];\n"
                "    int m_Ar2[11][10];\n"
                "    int* m_Ar3[10][20];\n"
                "    int* m_Ar4;\n"
                "    enum\n"
                "    {\n"
                "      NEW,\n"
                "      FIXED,\n"
                "      BROKEN,\n"
                "      READY\n"
                "    }* m_Ar5;\n"
                "    int** m_Ar6;\n"
                "    int**(* m_Ar7)[50];\n"
                "    int**(* m_Ar8[25])[50];\n"
                "    int**(*(* m_Ar9)[25])[50];\n"
                "  }* m_Madness[40];\n"
                "}"));
    assert( a.field( "m_Madness").element().element().field( "m_Ar9")== a.field( "m_Ar9"));
    assert( a.field( "m_Madness").element().element().field( "m_Ar9")!= a.field( "m_Ar8"));
    try {
        cout << ar2.field( "m_Foo")<< endl;
        assert( "field: missing exception!" == nullptr);
    } catch( const invalid_argument & e){
        assert( whitespaceMatch( e.what(), "Cannot use field() for type: int[11][10]"));
    }

    try {
        cout << c.element() << endl;
        assert( "element: missing exception!" == nullptr);
    } catch( const invalid_argument & e){
        assert( whitespaceMatch( e.what(), "Cannot use element() for type: struct\n"
                    "{\n"
                    "  int m_First;\n"
                    "  enum\n"
                    "  {\n"
                    "    NEW,\n"
                    "    FIXED,\n"
                    "    BROKEN,\n"
                    "    DEAD\n"
                    "  } m_Second;\n"
                    "  double m_Third;\n"
                    "}"));
    }
}
int main(void) {
    testsBasic();
    testsAdvanced();
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
