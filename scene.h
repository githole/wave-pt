#pragma once

#include "wavelength.h"

namespace wavept {

typedef wavept::Sphere<TSpectrum> TSphere;
TSphere spheres[] = {
	TSphere(10.0, wavept::Vec(80.0, 80.0, 20.0), TSpectrum(), TSpectrum(),    wavept::DIFFUSE), // 照明
	TSphere(30.0, wavept::Vec(50.0, 20.0, 0.0),  TSpectrum(), TSpectrum(0.7), wavept::DIFFUSE),  // 中央の球
	TSphere(1e5,  wavept::Vec(50, 1e5, 81.6),    TSpectrum(), TSpectrum(0.7), wavept::DIFFUSE),  // 中央の球
};
const int LightID = 0;

inline double light_radiance(const double wavelength) {
	return 32.0 / 95.0;
}


};