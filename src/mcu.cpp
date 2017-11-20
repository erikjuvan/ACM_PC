#include "mcu.h"

MCU::MCU(const std::string& port, int numOfChannels, int packetsPerChannel, int sampleFreq, int chBufSize) :
	serial_(port), numOfChannels_(numOfChannels), sampleFreq_(sampleFreq), chBufSize_(chBufSize) {

	serial_.flush();
	serial_.write(std::string("go"));
	serial_.write(std::to_string(numOfChannels));
	serial_.write(std::to_string(packetsPerChannel));
	serial_.write(std::to_string(sampleFreq));
}

MCU::~MCU() {
	if (serial_.isOpen()) {
		serial_.close();
	}
}
