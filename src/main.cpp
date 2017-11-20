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

#include "fft.h"
#include "mcu.h"
#include "console.h"
#include "parameters.h"

int main() {
	Parameters params;
	Console::initConsole();
	params.display();

	uint8_t* buffer = new uint8_t[params.chBufSize * params.nCh];

	FFT fft(params.nCh, params.sampleFreq, params.chBufSize, buffer);	
	fft.setOptimizationLevel(30);
	MCU mcu(params.comPort, params.nCh, params.packetsPerChannel, params.sampleFreq, params.chBufSize);	

	while (true) {
		if (mcu.readChunk(buffer) > 0) {
			fft.run();
			Console::gotoXY(0, 6);			
			fft.print();			
		}
	}

	delete[] buffer;

	return 0;
}
