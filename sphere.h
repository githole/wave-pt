#pragma once

#include "math.h"
#include "color.h"
#include "material.h"
#include "constant.h"

namespace wavept {
	
template <
	class ColorHolder
>
struct Sphere {
	double radius;
	Vec position;
	ColorHolder emission, color;
	ReflectionType ref_type;

	Sphere(
		const double radius_,
		const Vec &position_,
		const ColorHolder &emission_, 
		const ColorHolder &color_,
		const ReflectionType ref_type_) :
	  radius(radius_), position(position_), emission(emission_), color(color_), ref_type(ref_type_) {}

	// 入力のrayに対する交差点までの距離を返す。交差しなかったら0を返す。
	const double intersect(const Ray &ray) {
		Vec o_p = position - ray.org;
		const double b = Dot(o_p, ray.dir), det = b * b - Dot(o_p, o_p) + radius * radius;
		if (det >= 0.0) {
			const double sqrt_det = sqrt(det);
			const double t1 = b - sqrt_det, t2 = b + sqrt_det;
			if (t1 > EPS)		return t1;
			else if(t2 > EPS)	return t2;
		}
		return 0.0;
	}
};


};