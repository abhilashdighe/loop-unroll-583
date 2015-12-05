#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;
clock_t start;
ofstream outfile;

void startTimer() {
	start = clock();
	cout << "START: " << start << endl;
}

void endTimer(char *loopID) {
	double duration;
	clock_t end = clock();
	duration = (end - start) / (double) CLOCKS_PER_SEC;
	cout << "END: " << end << endl;
	cout << "CLOCKS_PER_SEC: " <<  (double) CLOCKS_PER_SEC << endl;
	cout << "DURATION: " << duration << endl;
	outfile.open("loopTimings.csv", ios_base::app);
	outfile << string(loopID) <<  ", " << duration << "\n";
}


