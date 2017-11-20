#pragma once

#include <vector>
#include <complex>
#include <fftw3.h>
#include <thread>
#include <mutex>

class FFT {
private:	

	struct FFTStruct {
		double* dataIn;
		std::complex<double>* dataOut;
		std::vector<fftw_plan> fftPlan;		

		double freq_, ampl_;
		int num_cycles_;

		FFTStruct(int bufSize);
		~FFTStruct();
	};
	
	std::vector<FFTStruct> fftChannels_;
	std::vector<std::thread> thread_;
	int numOfChannels_;
	int chBufSize_;
	int sampleFreq_;
	// Thread accessible parameters
	const uint8_t* buf_;

public:

	FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf);

	void print();
	void setOptimizationLevel(int optimizeLevel);
	void threadCompute(int ci);	
	void run();
};
