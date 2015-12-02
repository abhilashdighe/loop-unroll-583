#include <iostream>
#include <cstdio>
#include <ctime>

std::clock_t start;

void startTimer(int start) {
	start = std::clock();
}

int endTimer() {
	double duration;
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
}


