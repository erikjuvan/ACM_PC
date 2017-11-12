#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <complex>
#include <thread>

#include "FFT.h"
#include "MCU.h"
#include "Console.h"


class Parameters {
private:
	std::string sNCh, sPacketsPerChannel, sSampleFreq, sChBufSize;

	void clearScreen() {
#if defined(_WIN32)
		system("cls");
#elif defined(__linux__)
		system("clear");
#endif
	}

public:
	std::string comPort;
	int nCh, packetsPerChannel, sampleFreq, chBufSize;

	Parameters() : nCh(0), packetsPerChannel(0), sampleFreq(0), chBufSize(0) {
		set();
	}

	void set() {
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
			} else {
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

	void display() {
		clearScreen();
		std::cout << "COM Port: " << comPort << std::endl;
		std::cout << "Number of channels: " << sNCh << std::endl;
		std::cout << "Packets per channel: " << sPacketsPerChannel << std::endl;
		std::cout << "Sample frequency: " << sSampleFreq << " Hz" << std::endl;
		std::cout << "Channel buffer size: " << sChBufSize << std::endl;
	}
};

int main() {
	Parameters params;
	Console::initConsole();
	params.display();

	uint8_t* buffer = new uint8_t[params.chBufSize * params.nCh];
	std::vector<double> freq(params.nCh);
	std::vector<double> ampl(params.nCh);
	std::vector<int> cycles(params.nCh);

	FFT fft(params.nCh, params.sampleFreq, params.chBufSize, buffer, freq, ampl, cycles);	
	fft.setOptimizationLevel(30);
	MCU mcu(params.comPort, params.nCh, params.packetsPerChannel, params.sampleFreq, params.chBufSize);	

	while (true) {
		if (mcu.readChunk(buffer) > 0) {
			fft.run();
			Console::gotoXY(0, 6);			

			for (int i = 0; i < params.nCh; i++) {
				if (freq[i] >= 0.05) freq[i] -= 0.05;	// fix last digit error
				std::cout << "Ch " << i << "\tcycles: " <<  cycles[i] << "\tfreq: " << std::setw(4) << std::setprecision(1) << std::fixed << freq[i] << "\tamp: " <<ampl[i] << std::endl;
			}
		}
	}

	delete[] buffer;

	return 0;
}
