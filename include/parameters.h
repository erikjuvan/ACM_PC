#pragma once
#include <string>
#include <cstdlib>

class Parameters {
private:
	std::string sNCh, sPacketsPerChannel, sSampleFreq, sChBufSize;

	void clearScreen();

public:
	std::string comPort;
	int nCh, packetsPerChannel, sampleFreq, chBufSize;

	Parameters();
	void set();
	void display();
};
