#include "parameters.h"
#include <iostream>
#include <fstream>

void Parameters::clearScreen() {
#if defined(_WIN32)
	system("cls");
#elif defined(__linux__)
	system("clear");
#endif
}

Parameters::Parameters() : nCh(0), packetsPerChannel(0), sampleFreq(0), chBufSize(0) {
	set();
}

void Parameters::set() {
	bool succeed = false;
	std::ifstream initFile("../res/init.txt");

	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";
		std::getchar();
		clearScreen();

		initFile >> comPort;
		initFile >> sNCh;
		initFile >> sPacketsPerChannel;
		initFile >> sSampleFreq;
		initFile >> sChBufSize;

		if (sChBufSize.size() != 0) {
			nCh = std::stoi(sNCh);
			packetsPerChannel = std::stoi(sPacketsPerChannel);
			sampleFreq = std::stoi(sSampleFreq);
			chBufSize = std::stoi(sChBufSize);
			succeed = true;
		}
		else {
			std::cerr << "Error parsing init file" << std::endl;
		}
	}

	if (!succeed) {
		// Get parameters via command line
		std::cout << "COM Port: "; std::cin >> comPort;
		std::cout << "Number of channels: "; std::cin >> sNCh; nCh = std::stoi(sNCh);
		std::cout << "Packets per channel: "; std::cin >> sPacketsPerChannel; packetsPerChannel = std::stoi(sPacketsPerChannel);
		std::cout << "Sample frequency: "; std::cin >> sSampleFreq; sampleFreq = std::stoi(sSampleFreq);
		std::cout << "Channel buffer size: "; std::cin >> sChBufSize; chBufSize = std::stoi(sChBufSize);
	}
}

void Parameters::display() {
	clearScreen();
	std::cout << "COM Port: " << comPort << std::endl;
	std::cout << "Number of channels: " << sNCh << std::endl;
	std::cout << "Packets per channel: " << sPacketsPerChannel << std::endl;
	std::cout << "Sample frequency: " << sSampleFreq << " Hz" << std::endl;
	std::cout << "Channel buffer size: " << sChBufSize << std::endl;
}
