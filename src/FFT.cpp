#include <vector>
#include <complex>
#include <fftw3.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>

#include "FFT.h"

FFT::FFTStruct::FFTStruct(int bufSize) : freq_(0), ampl_(0), num_cycles_(0) {
	dataIn = new double[bufSize]();
	dataOut = new std::complex<double>[bufSize]();
	fftPlan.push_back(fftw_plan_dft_r2c_1d(bufSize, dataIn, reinterpret_cast<fftw_complex*>(dataOut), FFTW_MEASURE));
}

FFT::FFTStruct::~FFTStruct() {
	if (dataIn != nullptr) delete[] dataIn;
	if (dataOut != nullptr) delete[] dataOut;
	for (auto plan : fftPlan)
		fftw_destroy_plan(plan);
}

FFT::FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf) :
	numOfChannels_(numOfCh), sampleFreq_(sampleFreq), chBufSize_(bufSize),
	buf_(buf), runThread_(false), threadCpltCount_(0), thread_(numOfCh) {

	fftChannels_.reserve(numOfCh);
	for (int i = 0; i < numOfCh; i++) {
		fftChannels_.emplace_back(bufSize);		
	}		
}

void FFT::print() {
	int i = 0;
	for (auto& fftCh : fftChannels_) {
		if (fftCh.freq_ >= 0.05) fftCh.freq_ -= 0.05;	// fix last digit error
		std::cout << "Ch " << i++ << "\tcycles: " << fftCh.num_cycles_ << "\tfreq: " << std::setw(5) << std::setprecision(1) << std::fixed << fftCh.freq_ << "\tamp: " << fftCh.ampl_ << std::endl;
	}	
}

void FFT::setOptimizationLevel(int optimizeLevel) {
	// Clear all plans but the default one
	for (int chi = 0; chi < numOfChannels_; ++chi)
		while (fftChannels_[chi].fftPlan.size() > 1) fftChannels_[chi].fftPlan.pop_back();

	// Add optimization plans
	for (int chi = 0; chi < numOfChannels_; ++chi)
		for (int i = 1; i <= optimizeLevel; ++i)
			fftChannels_[chi].fftPlan.push_back(fftw_plan_dft_r2c_1d(chBufSize_ - i, fftChannels_[chi].dataIn, reinterpret_cast<fftw_complex*>(fftChannels_[chi].dataOut), FFTW_MEASURE));
}

void FFT::threadCompute(int ci) {
	FFTStruct& fftCh = fftChannels_[ci];

	// Setup data
	for (int i = 0; i < chBufSize_; ++i) {
		fftCh.dataIn[i] = static_cast<double>(*(buf_ + i * numOfChannels_ + ci));
	}

	int optimizeCnt = 0;
	double maxVal = 0.0;
	int idx = 0;
	int size = chBufSize_;
	for (auto plan : fftCh.fftPlan) {
		fftw_execute(plan);
		int tmpSize = chBufSize_ - optimizeCnt;
		optimizeCnt++;
		std::complex<double>* newMaxVal = std::max_element(fftCh.dataOut + 1, fftCh.dataOut + tmpSize / 4,	// discard 0 index (DC offset) and search only the first quarter
			[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });

		if (std::abs(*newMaxVal) > maxVal) {
			maxVal = std::abs(*newMaxVal);
			idx = std::distance(fftCh.dataOut, newMaxVal);
			size = tmpSize;
		}
	}

	// Return values
	fftCh.freq_ = (double)(idx * sampleFreq_) / (double)size;
	fftCh.ampl_ = maxVal * 2 / size;
	fftCh.num_cycles_ = static_cast<int>(idx);

	std::lock_guard<std::mutex> lock(cpltMutex);
	if (++threadCpltCount_ >= numOfChannels_) {
		runThread_ = false;
	}

}

void FFT::run() {
	int i = 0;

	for (auto& t: thread_) {
		t = std::thread(&FFT::threadCompute, this, i);
		i++;
	}

	for (auto& t : thread_) {
		t.join();
	}
}

