#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <map>
#include <set>

using namespace std;


int main (int argc, const char * argv[]) {
	// variables
	int expect_support = 3, expect_confidence = 65, read_int, n;	// the default value of expect_support and expect_confidence 
	string read_str;
	
	// make sure the input is 1 argument, which is only filename, or 3 arguments, which are filename, support and confidence
	// for example, ./a.out callgraph.txt  or  ./a.out callgraph.txt 10 80
	if(argc == 2 || argc == 4) {
		// if there are 3 arguments, update expect_support and expect_confidence
		if(argc == 4) {
			expect_support = atoi(argv[2]);
			expect_confidence = atoi(argv[3]);
		}
	} else {
		cerr << "Invalid input." << endl;
		exit(1);
	}
	
	// Note: since strings cannot to be operated efficiently, so use ID number to represent the functions
	map<string,int> func_list1;		// table from name to ID number
	
	// first time scan the file, look for all functions and give them IDs
	fstream fs;
	fs.open(argv[1]);
	if(fs.is_open()) {
		n = 0;
		while(fs.good() && !fs.eof()) {
			getline(fs, read_str);
			// collect the calling functions, give them function IDs and store them into tables
			if(read_str.find("Call graph node for function:") != string::npos) {
				string func_name = read_str.substr(read_str.find("'")+1,read_str.rfind("'")-read_str.find("'")-1);
				func_list1[func_name] = n;
				n++;
			}
		}
	}
	fs.close();
	
	const int num_func = func_list1.size();	// the number of functions
	// construct tables
	string* func_list2 = new string[num_func];	// table from number to ID name
	int* num_func_call = new int[num_func];	// table for how many times functions are called (support(function))
	for(int i = 0; i < num_func; i++) {
		num_func_call[i] = 0;
	}
	int** num_funcpair_call = new int*[num_func];	// table for how many times function pairs are called (support(function1, function2))
	for(int i = 0; i < num_func; i++) num_funcpair_call[i] = new int[num_func];
	for(int i = 0; i < num_func; i++) {
		for(int j = 0; j < num_func; j++) {
			num_funcpair_call[i][j] = 0;
		}
	}
	
	// second time scan the file, look for how many times the functions and function pairs are called
	fs.open(argv[1]);
	if(fs.is_open()) {
		n = 0;
		while(fs.good() && !fs.eof()) {
			getline(fs, read_str);
			// if read a function
			if(read_str.find("Call graph node for function:") != string::npos) {
				string func_name = read_str.substr(read_str.find("'")+1,read_str.rfind("'")-read_str.find("'")-1);
				func_list2[n] = func_name;	// map function IDs to function names
				n++;
				set<int> func_set;	// construct a set to store the functions which are called
				// while it is not the end of function
				while(read_str != "") {
					getline(fs, read_str);
					// if it is a function call
					if(read_str.find("calls function") != string::npos) {
						string func_name = read_str.substr(read_str.find("'")+1,read_str.rfind("'")-read_str.find("'")-1);
						func_set.insert(func_list1[func_name]);	// put it into the set
					}
				}
				// increase the number of function calls
				for(set<int>::iterator it = func_set.begin(); it != func_set.end(); it++) {
					num_func_call[*it]++;
				}
				// increase the number of function pairs calls
				for(set<int>::iterator it1 = func_set.begin(); it1 != func_set.end(); it1++) {
					for(set<int>::iterator it2 = it1; it2 != func_set.end(); it2++) {
						if(it2 != it1) num_funcpair_call[*it1][*it2]++;
					}
				}
			}
		}
	}
	fs.close();
	
	// third time scan, calculate support and confidence
	fs.open(argv[1]);
	if(fs.is_open()) {
		while(fs.good() && !fs.eof()) {
			getline(fs, read_str);
			// if read a function
			if(read_str.find("Call graph node for function:") != string::npos) {
				string callingfuncname = read_str.substr(read_str.find("'")+1,read_str.rfind("'")-read_str.find("'")-1);	// name of function is calling
				set<int> func_set;	// construct a set to store the functions which are called
				// while it is not the end of function
				while(read_str != "") {
					getline(fs, read_str);
					// if it is a function call
					if(read_str.find("calls function") != string::npos) {
						string func_name = read_str.substr(read_str.find("'")+1,read_str.rfind("'")-read_str.find("'")-1);
						func_set.insert(func_list1[func_name]);	// put it into the set
					}
				}
				
				// for each function which is called
				for(set<int>::iterator it = func_set.begin(); it != func_set.end(); it++) {
					int func1 = *it;
					// for each function which is not called
					for(int func2 = 0; func2 < num_func; func2++) {
						if(func_set.count(func2) == 0) {
							// if the actual support and actual confidence are not less than the expect support and expect confidence
							int actual_support;
							float actual_confidence;
							if(func1 < func2) {
								actual_support = num_funcpair_call[func1][func2];
								actual_confidence = 100.0*num_funcpair_call[func1][func2]/num_func_call[func1];
							} else {
								actual_support = num_funcpair_call[func2][func1];
								actual_confidence = 100.0*num_funcpair_call[func2][func1]/num_func_call[func1];
							}
							if(actual_support >= expect_support && actual_confidence >= expect_confidence) {
								//print out information
								string func1name = func_list2[func1], func2name = func_list2[func2];
								cout << "bug: " << func1name << " in " << callingfuncname << ", pair: (";
								// compare the alphabet order
								if (func1name < func2name) {
									cout << func1name << ", " << func2name;
								} else {
									cout << func2name << ", " << func1name;
								}
								cout << "), support: " << actual_support << ", ";
								cout.precision(2);	// set 2 decimal place
								cout << fixed << "confidence: " << actual_confidence << "%" << endl;
								cout.unsetf(ios::floatfield);	// reset default decimal place
							}
						}
					}
				}
			}
		}
	}
	fs.close();
	
	// free memory in heap
	delete [] func_list2;
	delete [] num_func_call;
	for(int i = 0; i < num_func; i++) {
		delete [] num_funcpair_call[i];
	}
	delete [] num_funcpair_call;
	
	return 0;
}

