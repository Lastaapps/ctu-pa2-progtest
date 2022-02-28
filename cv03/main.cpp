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

bool report ( const string & fileName, ostream & out ) {

	return false;
}

#ifndef __PROGTEST__
int main() {
	ostringstream oss;
	oss.str("");
	assert(report( "tests/test0_in.txt", oss ) == true);
	assert(oss.str() ==
			"John Christescu 258452362\n"
			"John Harmson 861647702\n"
			"-> 2\n"
			"-> 0\n"
			"Josh Dakhov 264112084\n"
			"Dakhov Speechley 865216101\n"
			"-> 2\n"
			"John Harmson 861647702\n"
			"-> 1\n" );
	oss.str("");
	assert(report( "tests/test1_in.txt", oss) == false);
	return 0;
}
#endif /* __PROGTEST__ */
