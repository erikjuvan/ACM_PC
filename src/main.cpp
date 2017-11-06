#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <complex>

#include "FFT.h"
#include "MCU.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__) 
#include<stdio.h>
#include<stdlib.h>
#endif


void InitParameters(std::string& sComPort, int& nCh, int& usPerSample, int& chBufSize) {
	std::string sNCh, sUsPerSample, sChBufSize;

	std::ifstream initFile("../res/init.txt");
	if (initFile.is_open()) {
		std::cout << "Found \"init.txt\" Press enter to load parameters from file.";		
		std::getchar();
#if defined(_WIN32)
		system("cls");
#elif defined(__linux__)
		system("clear");
#endif

		initFile >> sComPort;
		initFile >> sNCh;
		initFile >> sUsPerSample;
		initFile >> sChBufSize;
		nCh = std::stoi(sNCh);
		usPerSample = std::stoi(sUsPerSample);
		chBufSize = std::stoi(sChBufSize);

		std::cout << "COM Port: ";
		std::cout << sComPort << std::endl;
		std::cout << "Number of channels: ";
		std::cout << sNCh << std::endl;
		std::cout << "Us per sample: ";
		std::cout << sUsPerSample << std::endl;
		std::cout << "Single channel buffer size: ";
		std::cout << sChBufSize << std::endl;
	}
	else {
		std::cout << "COM Port: ";
		std::cin >> sComPort;
		std::cout << "Number of channels: ";
		std::cin >> sNCh;
		nCh = std::stoi(sNCh);
		std::cout << "Us per sample: ";
		std::cin >> sUsPerSample;
		usPerSample = std::stoi(sUsPerSample);
		std::cout << "Single channel buffer size: ";
		std::cin >> sChBufSize;
		chBufSize = std::stoi(sChBufSize);
	}
}

class Console {
#if defined(_WIN32)
	static HANDLE hStdout;
	static CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	static CONSOLE_CURSOR_INFO cursorInfo;
#endif

public:

	static bool InitConsole() {
#if defined(_WIN32)
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdout == INVALID_HANDLE_VALUE) {
			std::cout << "Error: GetStdHandle\n";
			return false;
		}
		if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
		{
			std::cout << "Error: GetConsoleScreenBufferInfo\n";
			return false;
		}

		GetConsoleCursorInfo(hStdout, &cursorInfo);
		cursorInfo.bVisible = false;
		SetConsoleCursorInfo(hStdout, &cursorInfo);
#endif
		return true;
	}

	static void GotoXY(int x, int y) {
#if defined(_WIN32)
		csbiInfo.dwCursorPosition.X = x;
		csbiInfo.dwCursorPosition.Y = y;
		SetConsoleCursorPosition(hStdout, csbiInfo.dwCursorPosition);
#elif defined(__linux__)
		printf("%c[%d;%df", 0x1B, y + 1, x);
#endif
	}
};

#if defined(_WIN32)
HANDLE Console::hStdout;
CONSOLE_SCREEN_BUFFER_INFO Console::csbiInfo;
CONSOLE_CURSOR_INFO	Console::cursorInfo;
#elif defined(__linux__)

#endif

void Help() {
	std::cout << "Buffer size directly influences resolution. Setting the size to 1000 (at 1ms sample rate) gives a 1Hz resolution (e.g. a 40 Hz result\
could be from a 39.5 to 40.4 Hz signal. Setting the size to 10000 gives a 0.1Hz resolution, and so on." << std::endl << std::endl;
}

int main() {			
	
	std::string sComPort;
	int nCh, usPerSample, chBufSize;

	Help();
	InitParameters(sComPort, nCh, usPerSample, chBufSize);
	
	Console::InitConsole();

	uint8_t* buffer = nullptr;
	buffer = new uint8_t[chBufSize * nCh];
	if (buffer == nullptr) {
		std::cout << "Couldn't allocate memory. Request of " << chBufSize * nCh << " bytes is too big" << std::endl;
	}

	FFT fft(nCh, chBufSize, usPerSample);
	MCU *mcu;
	try {
		mcu = new MCU(sComPort, nCh, chBufSize, usPerSample);
	}	
	catch (const serial::IOException& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
	catch (const std::bad_alloc& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	if (!mcu->IsSerialOpen()) {
		return -1;
	}

	const int MaxNumOfChannels = 10;
	double freq[MaxNumOfChannels] = { 0 };
	double ampl[MaxNumOfChannels] = { 0 };
	int cycles[MaxNumOfChannels] = { 0 };
	
	while (true) {
		if (mcu->ReadChunk((uint8_t*)buffer) > 0) {
			Console::GotoXY(0, 5);
			fft.Compute(buffer, freq, ampl, cycles);

			for (int i = 0; i < nCh; i++) {
				std::cout << "Ch " << i << "\tcycles: " << std::setw(4) << cycles[i] << "\tfreq: " << std::setw(4) << freq[i] << "\tamp: " << std::setw(4) << ampl[i] << std::endl;
			}
		}
	}

	if (buffer != nullptr) {
		delete[] buffer;
	}
	if (mcu != nullptr) {
		delete mcu;
	}

	return 0;
}
