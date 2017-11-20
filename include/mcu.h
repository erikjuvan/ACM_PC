#pragma once
#include <serial/serial.h>
#include <string>

class MCU {
private:
	serial::Serial serial_;
	int numOfChannels_;
	int sampleFreq_;
	int chBufSize_;

public:

	MCU(const std::string& port, int numOfChannels, int packetsPerChannel, int sampleFreq, int chBufSize);
	~MCU();

	size_t readChunk(uint8_t* data) { return serial_.read(data, numOfChannels_ * chBufSize_); }
	int getNumOfChannels() { return numOfChannels_; }
	int getSampleFreq() { return sampleFreq_; }
	size_t getRxBufferLen() { return serial_.available(); }
	bool isSerialOpen() { return serial_.isOpen(); }

};
