#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include <map>

using namespace std;
clock_t start;
ofstream outfile;

map<string, clock_t> loopStartTimes;

void startTimer(char *loopID) {
	start = clock();
	loopStartTimes[string(loopID)] = start;
}

void endTimer(char *loopID) {
	double duration;
	clock_t end = clock();
	duration = (end - loopStartTimes[string(loopID)]) / (double) CLOCKS_PER_SEC;
	loopStartTimes.erase(string(loopID));
	outfile.open("loopTimings.csv", ios_base::app);
	outfile << string(loopID) <<  ", " << duration << "\n";
	outfile.close();
}


