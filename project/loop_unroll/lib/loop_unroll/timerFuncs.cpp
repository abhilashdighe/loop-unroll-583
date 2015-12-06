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
const string OUTFILE = "loopTimings.csv";

inline bool exists(const string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

void startTimer(char *loopID) {
	// Print header in output file
	if(!exists(OUTFILE)) {
		outfile.open(OUTFILE, ios_base::app);
		outfile << "LoopID" <<  ", " << "Trip Count" << ", " << "Duration" << "\n";
		outfile.close();
	}
	start = clock();
	loopStartTimes[string(loopID)] = start;
}

void endTimer(char *loopID, int tripCount) {

	double duration;
	clock_t end = clock();
	duration = (end - loopStartTimes[string(loopID)]) / (double) CLOCKS_PER_SEC;
	loopStartTimes.erase(string(loopID));
	outfile.open(OUTFILE, ios_base::app);
	outfile << string(loopID) <<  ", " << tripCount << ", " << duration << "\n";
	outfile.close();
}
