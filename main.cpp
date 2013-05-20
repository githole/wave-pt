#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

#include "math.h"
#include "color.h"
#include "sphere.h"
#include "rand.h"
#include "hdr.h"
#include "wavelength.h"
#include "scene.h"
#include "radiance.h"

int main(int argc, char **argv) {
	const int width = 640;
	const int height = 480;
	const int samples = 32; // サンプル数

	// カメラ位置
	wavept::Ray camera(wavept::Vec(50.0, 52.0, 295.6), Normalize(wavept::Vec(0.0, -0.042612, -1.0)));
	// シーン内でのスクリーンのx,y方向のベクトル
	wavept::Vec cx = wavept::Vec(width * 0.5135 / height);
	wavept::Vec cy = Normalize(Cross(cx, camera.dir)) * 0.5135;
	wavept::RGBColor *image = new wavept::RGBColor[width * height];


	// 下で各波長についてパストレーシングするわけだが、波長の選択を完全にランダムにするのは効率が悪い。
	// 波長ごとに最終的な画像への寄与度は異なるからである。そこで、各波長の寄与度に基づいた重点サンプリングをする。
	// 具体的には各波長について、XYZ応答のうちY、すなわち輝度分に比例する確率密度関数を作り、それに基づいてサンプリングするようにする。
	// pdfはその密度関数でcdfはそこからサンプリングするための累積分布関数
	double cdf[wavept::NumWaveLengthTable];
	double pdf[wavept::NumWaveLengthTable];
	double luminance_table[wavept::NumWaveLengthTable];
	double total = 0.0;
	
	for (int i = 0; i < wavept::NumWaveLengthTable; i ++) {
		luminance_table[i] = wavept::Wavelength2XYZTable[i * 3 + 1];
	}
		
	for (int i = 0; i < wavept::NumWaveLengthTable; i ++) {
		total += luminance_table[i];
		cdf[i] = total;
		pdf[i] = luminance_table[i];
	}
	for (int i = 0; i < wavept::NumWaveLengthTable; i ++) {
		cdf[i] /= total;
		pdf[i] /= total;
	}
	
#pragma omp parallel for schedule(dynamic, 1) num_threads(7) // 7スレッド並列
	for (int y = 0; y < height; y ++) {
		unsigned int seeds[4];
		wavept::seed(seeds, y * y + y);

		std::cerr << "Rendering (" << samples * 4 << " spp) " << (100.0 * y / (height - 1)) << "%" << std::endl;
		for (int x = 0; x < width; x ++) {
			int image_index = y * width + x;	
			image[image_index] = wavept::RGBColor();

			wavept::RGBColor accumulated_radiance = wavept::RGBColor();
			// 一つのサブピクセルあたりsamples回サンプリングする
			for (int s = 0; s < samples; s ++) {
				wavept::Vec dir = cx * ((double)x / width - 0.5) + cy * ((double)y / height- 0.5) + camera.dir;
						 
				// 波長を重点サンプリング
				double* p = std::lower_bound(cdf, cdf + wavept::NumWaveLengthTable, wavept::rand01(seeds));
				int wavelength_index = p - cdf;
				if (wavelength_index >= wavept::NumWaveLengthTable) wavelength_index = wavept::NumWaveLengthTable - 1;
				const double div = pdf[wavelength_index];
				const double wavelength = (wavelength_index * 5 + 360); // nm
						
				// ラディアンスを計算
				const double value = radiance(seeds, wavept::Ray(camera.org + dir * 130.0, Normalize(dir)), 0, wavelength) / samples / div;
					
				// ある波長のラディアンスを得たら、それをXYZテーブルを使ってXYZへ変換、さらにxyz2rgbによってRGB値に変換する
				accumulated_radiance = accumulated_radiance + xyz2rgb(
					wavept::Vec( wavept::Wavelength2XYZTable[wavelength_index * 3], 
							wavept::Wavelength2XYZTable[wavelength_index * 3 + 1],
							wavept::Wavelength2XYZTable[wavelength_index * 3 + 2]) * value);
					
					
			}
			image[image_index] = image[image_index] + accumulated_radiance;
		}
	}

	/*
	// アンチエイリアス
#pragma omp parallel for schedule(dynamic, 1) num_threads(7) // 7スレッド並列
	for (int y = 0; y < height; y ++) {
		unsigned int seeds[4];
		wavept::seed(seeds, y * y + y);

		std::cerr << "Rendering (" << samples * 4 << " spp) " << (100.0 * y / (height - 1)) << "%" << std::endl;
		for (int x = 0; x < width; x ++) {
			int image_index = y * width + x;	
			image[image_index] = wavept::RGBColor();

			// 2x2のサブピクセルサンプリング
			for (int sy = 0; sy < 2; sy ++) {
				for (int sx = 0; sx < 2; sx ++) {
					wavept::RGBColor accumulated_radiance = wavept::RGBColor();
					// 一つのサブピクセルあたりsamples回サンプリングする
					for (int s = 0; s < samples; s ++) {
						// テントフィルターによってサンプリング
						// ピクセル範囲で一様にサンプリングするのではなく、ピクセル中央付近にサンプルがたくさん集まるように偏りを生じさせる
						const double r1 = 2.0 * wavept::rand01(seeds), dx = r1 < 1.0 ? sqrt(r1) - 1.0 : 1.0 - sqrt(2.0 - r1);
						const double r2 = 2.0 * wavept::rand01(seeds), dy = r2 < 1.0 ? sqrt(r2) - 1.0 : 1.0 - sqrt(2.0 - r2);
						wavept::Vec dir = cx * (((sx + 0.5 + dx) / 2.0 + x) / width - 0.5) +
								  cy * (((sy + 0.5 + dy) / 2.0 + y) / height- 0.5) + camera.dir;
						 
						// 波長を重点サンプリング
						double* p = std::lower_bound(cdf, cdf + wavept::NumWaveLengthTable, wavept::rand01(seeds));
						int wavelength_index = p - cdf;
						if (wavelength_index >= wavept::NumWaveLengthTable) wavelength_index = wavept::NumWaveLengthTable - 1;
						const double div = pdf[wavelength_index];
						const double wavelength = (wavelength_index * 5 + 360); // nm
						
						double value = radiance(seeds, wavept::Ray(camera.org + dir * 130.0, Normalize(dir)), 0, wavelength) / samples / div;
					
						accumulated_radiance = accumulated_radiance + xyz2rgb(
							wavept::RGBColor( wavept::Wavelength2XYZTable[wavelength_index * 3], 
							       wavept::Wavelength2XYZTable[wavelength_index * 3 + 1],
							       wavept::Wavelength2XYZTable[wavelength_index * 3 + 2]) * value);
					
					
					}
					image[image_index] = image[image_index] + accumulated_radiance;
				}
			}
		}
	}
	*/
	
	// .hdrフォーマットで出力
	wavept::save_hdr_file(std::string("image.hdr"), image, width, height);
}
