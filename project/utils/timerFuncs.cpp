#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

using namespace std;
clock_t start;
map<string, clock_t> loopStartTimes;
map<string, double> *loopExecTimes;
ofstream outfile;
char *outFileName;

class TimerUtils{
public:

	TimerUtils(char* fileName) {
		loopExecTimes = new map<string, double>;
		// Add header to timer output file
		outFileName = fileName;
		outfile.open(outFileName, ios_base::app);
		outfile << "LoopID" <<  ", " << ", " << "Duration" << "\n";
		outfile.close();
		atexit(write);
	}

	static void write(void) {
		// Write contents of loopTimes map to file
		outfile.open(outFileName, ios_base::app);
		for (auto& x : *loopExecTimes) {
			outfile << string(x.first) <<  ", " << x.second << "\n";
		}
		outfile.close();
		delete loopExecTimes;
	}

	void updateLoopTimes(char* loopID, double duration) {
		string strLoopID = string(loopID);
		if(loopExecTimes->find(strLoopID) == loopExecTimes->end()){
				(*loopExecTimes)[strLoopID] = 0;
		}
		loopExecTimes->at(strLoopID) += duration;
	}

};

TimerUtils objTimerUtils("loopTimings.csv");

void startTimer(char *loopID) {
	start = clock();
	loopStartTimes[string(loopID)] = start;
}

void endTimer(char *loopID, int tripCount) {

	if(loopStartTimes.find(string(loopID)) == loopStartTimes.end()){
		//cout << "Timer reset called without start" << endl;
		return;
		//} else {
		//cout << "All timers good!" <<endl;
	}

	double duration;
	clock_t end = clock();
	duration = (end - loopStartTimes[string(loopID)]) / (double) CLOCKS_PER_SEC;
	loopStartTimes.erase(string(loopID));
	objTimerUtils.updateLoopTimes(loopID, duration);
}
