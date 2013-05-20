#pragma once

#include "scene.h"

namespace wavept {

// 表面での挙動
// 各ベクトルはすべて物体表面上の点から出ていく向き
// 一般にinは視点方向のベクトルで、outは光源方向のベクトル
// wavelengthの単位はnm
// 各ベクトルはワールド座標系
double brdf(const int id, const wavept::Vec in, const wavept::Vec out, const wavept::Vec normal, const double wavelength) {
	const int index = wavept::wavelength2index((int)wavelength);

	// LambertianのBRDF
	return spheres[id].color.at(index) / wavept::PI;
};


};