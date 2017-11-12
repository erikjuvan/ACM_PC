#include "FFT.h"

#include <vector>
#include <complex>
#include <fftw3.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>

int FFT::FFTStruct::structCnt = 0;

FFT::FFTStruct::FFTStruct(int bufSize) : thread_(&FFT::threadCompute, this, structCnt) {
	structCnt++;
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

FFT::FFT(int numOfCh, int sampleFreq, int bufSize, const uint8_t* buf, std::vector<double>& freq, std::vector<double>& ampl, std::vector<int>& num_cycles) :
	numOfChannels_(numOfCh), sampleFreq_(sampleFreq), chBufSize_(bufSize),
	buf_(buf), freq_(freq), ampl_(ampl), num_cycles_(num_cycles),
	runThread_(false), threadCpltCount_(0) {
	fftChannels_.reserve(numOfCh);
	for (int i = 0; i < numOfCh; i++)
		fftChannels_.emplace_back(bufSize);
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

void FFT::threadCompute(FFT::FFTStruct* fftCh, int ci) {
	while (true) {
		if (runThread_) {

			// Setup data
			for (int i = 0; i < chBufSize_; ++i) {
				fftCh->dataIn[i] = static_cast<double>(*(buf_ + i * numOfChannels_ + ci));
			}

			int optimizeCnt = 0;
			double maxVal = 0.0;
			int idx = 0;
			int size = chBufSize_;
			for (auto plan : fftCh->fftPlan) {
				fftw_execute(plan);
				int tmpSize = chBufSize_ - optimizeCnt;
				optimizeCnt++;
				std::complex<double>* newMaxVal = std::max_element(fftCh->dataOut + 1, fftCh->dataOut + tmpSize / 4,	// discard 0 index (DC offset) and search only the first quarter
					[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });

				if (std::abs(*newMaxVal) > maxVal) {
					maxVal = std::abs(*newMaxVal);
					idx = std::distance(fftCh->dataOut, newMaxVal);
					size = tmpSize;
				}
			}

			// Return values
			freq_[ci] = (double)(idx * sampleFreq_) / (double)size;
			ampl_[ci] = maxVal * 2 / size;
			num_cycles_[ci] = static_cast<int>(idx);

			std::lock_guard<std::mutex> lock(cpltMutex);
			if (++threadCpltCount_ >= numOfChannels_) {
				runThread_ = false;
			}
		}
	}
}

void FFT::run() {
	runThread_ = true;
}

