#pragma once

#include "math.h"

namespace wavept {
	
// RGB
typedef Vec RGBColor;

// スペクトル
// 単位はnm
template <
	int NumSpectrum,		// 保存される波長の数
	int FirstWavelength,	// 保存される、最初の波長
	int IntervalWavelength	// 保存される一つ一つの波長の間隔
>
class Spectrum {
private:
	double power[NumSpectrum];
public:
	typedef Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength> SelfType;

	enum TemplateParameter {
		NumSpectrum = NumSpectrum,
		FirstWavelength = FirstWavelength,
		IntervalWavelength = IntervalWavelength,
	};

	Spectrum(void) {
		for (int i = 0; i < NumSpectrum; ++i )
			power[i] = 0.0;
	}

	Spectrum(const double value) {
		for (int i = 0; i < NumSpectrum; ++i )
			power[i] = value;
	}

	double at(const int index) const {
		if (index < 0 || NumSpectrum <= index) {
			return -1.0;
		}
		return power[index];
	}

	void set(const int index, const double value) {
		if (index < 0 || NumSpectrum <= index) {
			return;
		}
		power[index] = value;
	}

	inline SelfType operator+(const SelfType &b) const {
		SelfType tmp;
		for (int i = 0; i < NumSpectrum; ++i)
			tmp.power[i] = power[i] + b.power[i];
		return tmp;
	}
	inline SelfType operator-(const SelfType &b) const {
		SelfType tmp;
		for (int i = 0; i < NumSpectrum; ++i)
			tmp.power[i] = power[i] - b.power[i];
		return tmp;
	}
	inline SelfType operator*(const double b) const {
		SelfType tmp;
		for (int i = 0; i < NumSpectrum; ++i)
			tmp.power[i] = power[i] * b;
		return tmp;
	}
	inline SelfType operator/(const double b) const {
		SelfType tmp;
		for (int i = 0; i < NumSpectrum; ++i)
			tmp.power[i] = power[i] / b;
		return tmp;
	}
};


template <int NumSpectrum, int FirstWavelength, int IntervalWavelength>
inline Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength>
operator*(double f, const Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength> &v) { 
	return v * f; 
}

template <int NumSpectrum, int FirstWavelength, int IntervalWavelength>
inline Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength>
operator*(
const Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength> &v1,
const Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength> &v2) { 
	Spectrum<NumSpectrum, FirstWavelength, IntervalWavelength> tmp;
	for (int i = 0; i < tmp.NumSpectrum; ++i) {
		tmp.set(i, v1.at(i) * v2.at(i));
	}
	return tmp;
}

};