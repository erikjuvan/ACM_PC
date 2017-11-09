#pragma once
#include <serial/serial.h>
#include <string>
#include <iostream>

class MCU {
private:
	serial::Serial serial_;
	int numOfChannels_;
	int sampleFreq_;
	int chBufSize_;

public:

	MCU(const std::string& port, int numOfChannels, int chBufSize, int sampleFreq) :
		serial_(port), numOfChannels_(numOfChannels), chBufSize_(chBufSize), sampleFreq_(sampleFreq) {

			serial_.flush();
			serial_.write(std::string("go"));
			serial_.write(std::to_string(numOfChannels));
			serial_.write(std::to_string(sampleFreq));
	}

	~MCU() {
		if (serial_.isOpen()) {
			serial_.close();
		}
	}

	size_t readChunk(uint8_t* data) {
		return serial_.read(data, numOfChannels_ * chBufSize_);
	}

	int getNumOfChannels() { return numOfChannels_; }
	int getSampleFreq() { return sampleFreq_; }
	size_t getRxBufferLen() { return serial_.available(); }
	bool isSerialOpen() { return serial_.isOpen(); }

};
