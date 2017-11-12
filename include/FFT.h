#pragma once

#include <vector>
#include <complex>
#include <fftw3.h>
#include <thread>
#include <mutex>

class FFT {
private:

	struct FFTStruct {
		static int structCnt;

		double* dataIn;
		std::complex<double>* dataOut;
		std::vector<fftw_plan> fftPlan;
		std::thread	thread_;

		FFTStruct(int bufSize);
		~FFTStruct();
	};

	std::vector<FFTStruct> fftChannels_;
	int numOfChannels_;
	int chBufSize_;
	int sampleFreq_;
	// Multi-threading
	bool runThread_;
	int threadCpltCount_;
	std::mutex cpltMutex;
	// Thread accessible parameters
	const uint8_t* buf_;
	std::vector<double>& freq_;
	std::vector<double>& ampl_;
	std::vector<int>& num_cycles_;

public:

	FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf, std::vector<double>& freq, std::vector<double>& ampl, std::vector<int>& num_cycles);

	void setOptimizationLevel(int optimizeLevel);
	void threadCompute(FFTStruct* st, int ci);	
	void run();
};
