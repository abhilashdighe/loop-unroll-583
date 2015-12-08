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
char *OUTFILE = "loopTimings.csv";

inline bool exists(const char* name) {
    ifstream f(name);
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

	if(loopStartTimes.find(string(loopID)) == loopStartTimes.end()){
		cout << "Timer reset called without start" << endl;
		return;
	}

	double duration;
	clock_t end = clock();
	duration = (end - loopStartTimes[string(loopID)]) / (double) CLOCKS_PER_SEC;
	loopStartTimes.erase(string(loopID));
	outfile.open(OUTFILE, ios_base::app);
	outfile << string(loopID) <<  ", " << tripCount << ", " << duration << "\n";
	outfile.close();
}
