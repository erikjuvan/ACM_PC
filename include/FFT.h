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
		fftw_plan fftPlan;

		FFTStruct(int bufSize) {
			dataIn = new double[bufSize]();
			dataOut = new std::complex<double>[bufSize]();
			fftPlan = fftw_plan_dft_r2c_1d(bufSize, dataIn, reinterpret_cast<fftw_complex*>(dataOut), FFTW_MEASURE);
		}

		~FFTStruct() {
			if (dataIn != nullptr) delete[] dataIn;
			if (dataOut != nullptr) delete[] dataOut;
			fftw_destroy_plan(fftPlan);
		}
	};

	int numOfChannels_;
	int chBufSize_;
	int sampleFreq_;

	std::vector<FFTStruct> fftChannels_;

	void Optimize(FFTStruct& fftCh, double& maxVal, double& idx, int& size) {
		size = chBufSize_;
		int tmpSize = size;
		int target = size - 50;
		int i = 0;

		while (--tmpSize > target) {
			fftCh.fftPlan = fftw_plan_dft_r2c_1d(tmpSize, fftCh.dataIn, reinterpret_cast<fftw_complex*>(fftCh.dataOut), FFTW_MEASURE);
			// Run FFT
			fftw_execute(fftCh.fftPlan);
			// Find max element - optimized
			std::complex<double>* newMaxVal = std::max_element(fftCh.dataOut + 1, fftCh.dataOut + tmpSize / 2,	// discard 0 index (DC offset) and search only the first half
				[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });			

			if (std::abs(*newMaxVal) > maxVal) {
				maxVal = std::abs(*newMaxVal);				
				idx = std::distance(fftCh.dataOut, newMaxVal);
				size = tmpSize;
			}
		}	
	}

public:

	FFT(int numOfCh, int bufSize, int sampleFreq) : numOfChannels_(numOfCh), chBufSize_(bufSize), sampleFreq_(sampleFreq) {
		fftChannels_.reserve(numOfCh);
		for (int i = 0; i < numOfCh; i++)
			fftChannels_.emplace_back(bufSize);
	}

	~FFT() { }

	void Compute(const uint8_t* buf, double* freq, double* ampl, int* num_cycles) {

		for (int ci = 0; ci < numOfChannels_; ++ci) {

			// Setup data
			for (int i = 0; i < chBufSize_; ++i) {
				fftChannels_[ci].dataIn[i] = static_cast<double>(*(buf + i * numOfChannels_ + ci));
			}

			// Run FFT
			fftw_execute(fftChannels_[ci].fftPlan);	

			// Find max element
			std::complex<double>* maxVal = std::max_element(fftChannels_[ci].dataOut + 1, fftChannels_[ci].dataOut + chBufSize_ / 2,	// discard 0 index (DC offset) and search only the first half
				[](std::complex<double> const & lhs, std::complex<double> const & rhs) { return std::abs(lhs) < std::abs(rhs); });
			double idx = std::distance(fftChannels_[ci].dataOut, maxVal);

			// Optimize
			int fftSize = chBufSize_;
			double maxRealVal = std::abs(*maxVal);
			Optimize(fftChannels_[ci], maxRealVal, idx, fftSize);

			// Return values
			freq[ci] = (double)(idx * sampleFreq_) / (double)fftSize;
			ampl[ci] = maxRealVal * 2.0 / fftSize;
			if (num_cycles != nullptr) {
				num_cycles[ci] = static_cast<int>(idx);
			}
		}
	}
};
