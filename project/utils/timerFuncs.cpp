#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include <map>

using namespace std;
clock_t start;
map<string, clock_t> loopStartTimes;

class TimerUtils{

	ofstream outfile;
	char *outFileName;
    // Loopid vs execution time
	map<string, double> *loopExecTimes;

public:

	TimerUtils(char* outFileName) {
		loopExecTimes = new map<string, double>;
		// Add header to timer output file
		this -> outFileName = outFileName;
		outfile.open(outFileName, ios_base::app);
		outfile << "LoopID" <<  ", " << "Trip Count" << ", " << "Duration" << "\n";
		outfile.close();
	}

	~TimerUtils() {
		// Write contents of loopTimes map to file
		outfile.open(outFileName, ios_base::app);
		for (auto& x : *loopExecTimes) {
			outfile << string(x.first) <<  ", " << x.second << "\n";
			cout << x.second << endl;
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
		cout << loopExecTimes->at(strLoopID) << endl;
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
