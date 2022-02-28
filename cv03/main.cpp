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

#define DEBUG
using namespace std;
#endif /* __PROGTEST__ */

struct Contact {
	size_t order;
	string firstName, lastName, number;
};

bool openFile(const string & fileName, ifstream & in);
bool readLines(ifstream & in, vector<Contact> & vector);

bool report ( const string & fileName, ostream & out ) {

	// opening file
	ifstream in;
	bool succeeded = openFile(fileName, in);
	#ifdef DEBUG
	cout << "File opened: " << succeeded << endl;
	#endif
	if (!succeeded) return false;

	vector<Contact> contacts;
	if (!readLines(in, contacts)) {
#ifdef DEBUG
		cout << "Reading error" << endl;
#endif
		return false;
	}

	for (auto & c : contacts) {
		cout << c.order << " " << c.firstName << " " << c.lastName << " " << c.number << "\n";
	}
	cout << flush;

	in.close();
	return true;
}

bool openFile(const string & fileName, ifstream & in) {
	in = ifstream(fileName, ios::in);
	return in.is_open();
}

bool readLines(ifstream & in, vector<Contact> & vector) {
	string line;
	stringstream str;

	for (size_t order = 0;in; order++) {
		getline(in, line);
		#ifdef DEBUG
		cout << "Line read: " << line << endl;
		#endif
		if (line.length() == 0) break;

		string first, last, number, remaining;
		str.clear();
		str.str(line);
		str >> first >> last >> number;
		//TODO check number

		str >> remaining;
		if (remaining.length() != 0) {
#ifdef DEBUG
			cout << "Remaining found" << endl;
#endif
			return false;
		}
		vector.push_back({order, first, last, number});
	}
	return in.good();
}

bool checkPhoneNumber(string & number) {
	if (number.length() != 9) return false;
	for (auto c : number)
		if (c < '0' || '9' < c) return false;
	return number.at(0) != '0';
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
