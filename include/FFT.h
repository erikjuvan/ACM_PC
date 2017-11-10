#pragma once

#include <vector>
#include <complex>
#include <fftw3.h>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <chrono>

class FFT {
private:

	struct FFTStruct {
		double* dataIn;
		std::complex<double>* dataOut;
		std::vector<fftw_plan> fftPlan;

		FFTStruct(int bufSize) {
			dataIn = new double[bufSize]();
			dataOut = new std::complex<double>[bufSize]();
			fftPlan.push_back(fftw_plan_dft_r2c_1d(bufSize, dataIn, reinterpret_cast<fftw_complex*>(dataOut), FFTW_MEASURE));
		}

		~FFTStruct() {
			if (dataIn != nullptr) delete[] dataIn;
			if (dataOut != nullptr) delete[] dataOut;
			for (auto plan : fftPlan)
				fftw_destroy_plan(plan);
		}
	};

	std::vector<FFTStruct> fftChannels_;
	int numOfChannels_;
	int chBufSize_;
	int sampleFreq_;	

public:

	FFT(int numOfCh, int sampleFreq, int bufSize) : numOfChannels_(numOfCh), sampleFreq_(sampleFreq), chBufSize_(bufSize) {
		fftChannels_.reserve(numOfCh);
		for (int i = 0; i < numOfCh; i++)
			fftChannels_.emplace_back(bufSize);
	}

	~FFT() { }

	void setOptimizationLevel(int optimizeLevel) {
		// Clear all plans but the default one
		for (int chi = 0; chi < numOfChannels_; ++chi)
			while (fftChannels_[chi].fftPlan.size() > 1) fftChannels_[chi].fftPlan.pop_back();

		// Add optimization plans
		for (int chi = 0; chi < numOfChannels_; ++chi)
			for (int i = 1; i <= optimizeLevel; ++i)
				fftChannels_[chi].fftPlan.push_back(fftw_plan_dft_r2c_1d(chBufSize_ - i, fftChannels_[chi].dataIn, reinterpret_cast<fftw_complex*>(fftChannels_[chi].dataOut), FFTW_MEASURE));	
	}

	void run(const uint8_t* buf, std::vector<double>& freq, std::vector<double>& ampl, std::vector<int>& num_cycles) {
		for (int ci = 0; ci < numOfChannels_; ++ci) {
			// Setup data
			for (int i = 0; i < chBufSize_; ++i) {
				fftChannels_[ci].dataIn[i] = static_cast<double>(*(buf + i * numOfChannels_ + ci));
			}

			int optimizeCnt = 0;
			double maxVal = 0.0;
			int idx = 0;
			int size = chBufSize_;
			for (auto plan : fftChannels_[ci].fftPlan) {
				fftw_execute(plan);
				int tmpSize = chBufSize_ - optimizeCnt;
				optimizeCnt++;
				std::complex<double>* newMaxVal = std::max_element(fftChannels_[ci].dataOut + 1, fftChannels_[ci].dataOut + tmpSize / 4,	// discard 0 index (DC offset) and search only the first quarter
					[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });				

				if (std::abs(*newMaxVal) > maxVal) {
					maxVal = std::abs(*newMaxVal);
					idx = std::distance(fftChannels_[ci].dataOut, newMaxVal);
					size = tmpSize;
				}
			}

			// Return values
			freq[ci] = (double)(idx * sampleFreq_) / (double)size;
			ampl[ci] = maxVal * 2 / size;
			num_cycles[ci] = static_cast<int>(idx);
		}
	}
};
