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
class CDataTypeInt {
    // todo
};
class CDataTypeDouble {
    // todo
};
class CDataTypeEnum {
    // todo
};
class CDataTypeStruct {
    // todo
};
#ifndef __PROGTEST__
static bool whitespaceMatch(const string & a,
        const string & b){
    // todo
    return true;
}
template <typename T_>
static bool whitespaceMatch(const T_ & x,
        const string & ref) {
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
