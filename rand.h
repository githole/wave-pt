#pragma once

#include <cstdlib>

namespace wavept {



unsigned int xor128(unsigned int seeds[4]) { 
  static unsigned int &x = seeds[0];
  static unsigned int &y = seeds[1];
  static unsigned int &z = seeds[2];
  static unsigned int &w = seeds[3]; 
  unsigned int t;
 
  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)); 
}

void seed(unsigned int seeds[4], unsigned int s){    
	for (int i = 1; i <= 4; i++) {
		seeds[i - 1] = s = 1812433253U * (s^(s>>30)) + i;
	}
}

inline double rand01(unsigned int seeds[4]) { 
	return (double)xor128(seeds)/UINT_MAX; 
}


};