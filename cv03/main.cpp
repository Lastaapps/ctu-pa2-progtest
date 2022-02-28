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

#define DPRINT if (true)


struct Contact {
	string firstName, lastName, number;
};

bool openFile(const string &fileName, ifstream &in);
bool readLines(ifstream &in, vector<Contact> &vector);
bool checkPhoneNumber(const string &number);
bool search(ostream & out, ifstream & in, const vector<Contact> & vector);
void printForQuery(ostream & out, const string & query, const vector<Contact> & vector);


bool report(const string &fileName, ostream &out) {

	DPRINT cout << "\n\nWorking on " << fileName << "\n" << endl;
	// opening file
	ifstream in;
	bool succeeded = openFile(fileName, in);
	DPRINT cout << "File opened: " << succeeded << endl;
	if (!succeeded) return false;

	vector<Contact> contacts;
	if (!readLines(in, contacts))
	{
		DPRINT cout << "Reading error" << endl;
		in.close();
		return false;
	}

#ifdef DEBUG
	cout << "Contacts read:" << endl;
	for (auto &c : contacts) {
		cout << c.firstName << " " << c.lastName << " " << c.number << "\n";
	}
	cout << endl;
#endif

	bool searchResult = search(out, in, contacts);

	in.close();
	return searchResult;
}

bool openFile(const string &fileName, ifstream &in) {
	in = ifstream(fileName, ios::in);
	return in.is_open() && in.good();
}

bool readLines(ifstream & in, vector<Contact> & vector) {
	string line;
	stringstream str;

	while (in) {
		getline(in, line);
		if (line.length() == 0)
			break;

		string first, last, number, remaining;
		str.clear();
		str.str(line);
		str >> first >> last >> number;
		if (first.length() == 0 || last.length() == 0 || !checkPhoneNumber(number)) {
			DPRINT cout << "Remaining found" << endl;
			return false;
		}

		str >> remaining;
		if (remaining.length() != 0) {
			DPRINT cout << "Remaining found" << endl;
			return false;
		}
		vector.push_back({first, last, number});
	}
	return in.good();
}

bool checkPhoneNumber(const string &number) {
	if (number.length() != 9)
		return false;
	for (auto c : number)
		if (c < '0' || '9' < c)
			return false;
	return number.at(0) != '0';
}

bool search(ostream & out, ifstream & in, const vector<Contact> & vector) {
	for (; in; ) {
		//auto peaked = in.peek();
	 	//cout << "Peaked " << peaked << " (" << (int) peaked << ")" << endl;
		in.peek();
		if (in.eof()) break;

		string line = "", query = "";

		getline(in, line);
		stringstream str(line);
		str >> query;

		//cout << "line: '" << line << "' query: '" << query << "'" << endl;
		//cout << "good: " << in.good() << " eof: " << in.eof() << endl;
		//cout << "good: " << str.good() << " eof: " << str.eof() << endl;

		if (in.good() || in.eof())
			printForQuery(out, query, vector);
	}
	return in.good() || in.eof();
}

void printForQuery(ostream & out, const string & query, const vector<Contact> & vector) {
	DPRINT cout << "Quering: '" << query << "'" << endl;
	size_t counter = 0;
	for (auto & c : vector) {
		if (c.firstName.compare(query) == 0 || c.lastName.compare(query) == 0 ) {
			counter++;
			out << c.firstName << " " << c.lastName << " " << c.number << "\n";
			DPRINT cout << c.firstName << " " << c.lastName << " " << c.number << endl;
		}
	}
	out << "-> " << counter << endl;
	DPRINT cout << "-> " << counter << endl;
}






#ifndef __PROGTEST__
int main() {
	ostringstream oss;
	oss.str("");
	assert(report("tests/test0_in.txt", oss) == true);
	assert(oss.str() ==
		   "John Christescu 258452362\n"
		   "John Harmson 861647702\n"
		   "-> 2\n"
		   "-> 0\n"
		   "Josh Dakhov 264112084\n"
		   "Dakhov Speechley 865216101\n"
		   "-> 2\n"
		   "John Harmson 861647702\n"
		   "-> 1\n");
	oss.str("");
	assert(report("tests/test1_in.txt", oss) == false);

	oss.str("");
	assert(report("tests/progtest0.txt", oss));
	assert(oss.str() ==
			"Carly Johnson 355562144\n"
		    "-> 1\n"
			"Christen Day 711567172\n"
		    "-> 1\n");

	oss.str("");
	assert(report("tests/progtest1.txt", oss));
	assert(oss.str() ==
		   "John Christescu 258452362\n"
		   "John Christescu 258452362\n"
		   "John Christescu 258452362\n"
		   "-> 3\n"
		   "-> 0\n"
		   "-> 0\n"
		   "-> 0\n"
		   "-> 0\n");

	return 0;
}
#endif /* __PROGTEST__ */
