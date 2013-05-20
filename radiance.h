#pragma once

#include "scene.h"
#include "brdf.h"

namespace wavept {

static const double MaxDepth = 5;
static const bool IndirectLight = true;

// *** レンダリング用関数 ***
// シーンとの交差判定関数
inline bool intersect_scene(const wavept::Ray &ray, double *t, int *id) {
	const double n = sizeof(spheres) / sizeof(TSphere);
	*t  = wavept::INF;
	*id = -1;
	for (int i = 0; i < int(n); i ++) {
		double d = spheres[i].intersect(ray);
		if (d > 0.0 && d < *t) {
			*t  = d;
			*id = i;
		}
	}
	return *t < wavept::INF;
}

// 光源上の点をサンプリングして直接光を計算する
double direct_radiance_sample(unsigned int seeds[4], const wavept::Vec &v0, const wavept::Vec &in, const wavept::Vec &normal, const int id,  const double wavelength) {
	// 光源上の一点をサンプリングする
	const double r1 = 2 * wavept::PI * wavept::rand01(seeds);
	const double r2 = 1.0 - 2.0 * wavept::rand01(seeds);
	const wavept::Vec light_pos = spheres[LightID].position + ((spheres[LightID].radius + wavept::EPS) * wavept::Vec(sqrt(1.0 - r2*r2) * cos(r1), sqrt(1.0 - r2*r2) * sin(r1), r2));
	
	// サンプリングした点から計算
	const wavept::Vec light_normal = Normalize(light_pos - spheres[LightID].position);
	const wavept::Vec light_dir = Normalize(light_pos - v0);
	const double dist2 = (light_pos - v0).LengthSquared();
	const double dot0 =  Dot(normal, light_dir);
	const double dot1 = Dot(light_normal, -1.0 * light_dir);

	if (dot0 >= 0 && dot1 >= 0) {
		const double G = dot0 * dot1 / dist2;
		double t; // レイからシーンの交差 位置までの距離
		int id_; // 交差したシーン内オブジェクトのID
		intersect_scene(wavept::Ray(v0, light_dir), &t, &id_);
		if (fabs(sqrt(dist2) - t) < 1e-3) {	
			const double emission = light_radiance(wavelength);
			return emission * brdf(id, in, light_dir, normal, wavelength)
				 * G / (1.0 / (4.0 * wavept::PI * pow(spheres[LightID].radius, 2.0)));
		}
	}
	return 0.0;
}


// ray方向からの放射輝度を求める
// wavelength (nm)
double radiance(unsigned int seeds[4], const wavept::Ray &ray, const int depth, const double wavelength) {
	double t; // レイからシーンの交差位置までの距離
	int id;   // 交差したシーン内オブジェクトのID
	if (!intersect_scene(ray, &t, &id)) {
		return 0.0;
	}
	const TSphere &obj = spheres[id];
	const wavept::Vec hitpoint = ray.org + t * ray.dir; // 交差位置
	const wavept::Vec normal  =  Normalize(hitpoint - obj.position); // 交差位置の法線
	wavept::Vec orienting_normal = Dot(normal, ray.dir) < 0.0 ? normal : (-1.0 * normal); // 交差位置の法線（物体からのレイの入出を考慮）
	// 色の反射率最大のものを得る。ロシアンルーレットで使う。
	// ロシアンルーレットの閾値は任意だが色の反射率等を使うとより良い。
	double russian_roulette_probability = obj.color.at(0);
	double hoge = wavept::rand01(seeds);
	// 一定以上レイを追跡したらロシアンルーレットを実行し追跡を打ち切るかどうかを判断する
	if (depth > MaxDepth) {
		if (hoge >= russian_roulette_probability) {
			return 0.0;
		}
	} else
		russian_roulette_probability = 1.0; // ロシアンルーレット実行しなかった
	
	switch (obj.ref_type) {
	case wavept::DIFFUSE: {
		/*
		// implicit version
		// orienting_normalの方向を基準とした正規直交基底(w, u, v)を作る。この基底に対する半球内で次のレイを飛ばす。
		wavept::Vec w, u, v;
		w = orienting_normal;
		if (fabs(w.x) > 0.1)
			u = Normalize(Cross(wavept::Vec(0.0, 1.0, 0.0), w));
		else
			u = Normalize(Cross(wavept::Vec(1.0, 0.0, 0.0), w));
		v = Cross(w, u);

		// コサイン項を使った重点的サンプリング
		const double r1 = 2 * wavept::PI * wavept::rand01(seeds);
		const double r2 = wavept::rand01(seeds), r2s = sqrt(r2);
		wavept::Vec dir = Normalize((u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)));

		const double emission = id == LightID ? light_radiance(wavelength) : 0.0;
		const wavept::Vec in = Normalize(-1.0 * ray.dir);
		if (depth == 0) {
			return wavept::PI * brdf(id, in, dir, orienting_normal, wavelength)
					* radiance(seeds, wavept::Ray(hitpoint, dir), depth+1, wavelength) / russian_roulette_probability;
		} else {
			if (IndirectLight) {
				return emission + wavept::PI * brdf(id, in, dir, orienting_normal, wavelength)
					* radiance(seeds, wavept::Ray(hitpoint, dir), depth+1, wavelength) / russian_roulette_probability;
			} else {
				return emission;
			}
		}
		*/
		// explicit version
		if (id != LightID) {
			const int shadow_ray = 1;
			double direct_light(0.0);
			const wavept::Vec in = Normalize(-1.0 * ray.dir);
			for (int i = 0; i < shadow_ray; i ++) {
				direct_light = direct_light + direct_radiance_sample(seeds, hitpoint, in, orienting_normal, id, wavelength) / shadow_ray;
			}

			// orienting_normalの方向を基準とした正規直交基底(w, u, v)を作る。この基底に対する半球内で次のレイを飛ばす。
			wavept::Vec w, u, v;
			w = orienting_normal;
			if (fabs(w.x) > 0.1)
				u = Normalize(Cross(wavept::Vec(0.0, 1.0, 0.0), w));
			else
				u = Normalize(Cross(wavept::Vec(1.0, 0.0, 0.0), w));
			v = Cross(w, u);
			// コサイン項を使った重点的サンプリング
			const double r1 = 2 * wavept::PI * wavept::rand01(seeds);
			const double r2 = wavept::rand01(seeds), r2s = sqrt(r2);
			wavept::Vec dir = Normalize((u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0 - r2)));

			if (IndirectLight)
				return (direct_light + wavept::PI * brdf(id, wavept::Vec(), wavept::Vec(), wavept::Vec(), wavelength)
					* radiance(seeds, wavept::Ray(hitpoint, dir), depth+1, wavelength)) / russian_roulette_probability;
			else
				return direct_light;
		} else if (depth == 0) {
			// 光源をレンダリングしない
			return 0.0;
//			return light_radiance(wavelength);
		} else
			return 0.0;

	} break;
	}

	return 0.0;
}
};