#include "pch.h"
#include <iostream>
#include <set>
#include <random>
#include <cassert>

class UniqCounter {
	// no more than 32kb of memory should be used here

public:
	UniqCounter() {
		register_m = new unsigned char[m];
		for (int i = 0; i < m; ++i) {
			register_m[i] = 0;
		}
	}

	void add(int x) {
		unsigned x_hash = int_murmur_hash(&x);
		unsigned id_x = x_hash >> k_complement;
		int czb = count_zero_bits(x_hash);
		int rank = k_complement < czb ? k_complement + 1 : czb + 1;
		register_m[id_x] = register_m[id_x] > rank ? register_m[id_x] : (unsigned char)rank;
	}

	int get_uniq_num() const {
		double c = 0.0;
		for (int i = 0; i < m; ++i) {
			c += 1.0 / pow(2.0, register_m[i]);
		}
		int estimate = alpha_m2 / c;
		if (estimate <= (2.5 * m)) {
			int v = 0;
			for (int i = 0; i < m; ++i) {
				if (register_m[i] == 0) { 
					++v; 
				}
			}
			if (v > 0) {
				estimate = m * log((double)m / v);
			}
		}
		else if (estimate > (0x100000000 / 30.0)) {
			estimate = -0x100000000 * log(1.0 - (estimate / 0x100000000));
		}

		return estimate;
	}

private:
	int k_complement = 21;
	int m = 2048;
	double alpha_m2 = 3023758.3915552306;
	unsigned char *register_m;

	int count_zero_bits(int v) {
		if ((v & 0x1) == 1) {
			return 0; 
		}
		int c = 1;
		if ((v & 0xffff) == 0) { 
			v >>= 16; 
			c += 16; 
		}
		if ((v & 0xff) == 0) {
			v >>= 8; 
			c += 8; 
		}
		if ((v & 0xf) == 0) { 
			v >>= 4; 
			c += 4; 
		}
		if ((v & 0x3) == 0) { 
			v >>= 2; 
			c += 2;
		}
		c -= v & 0x1;
		
		return c;
	}
	
	unsigned int_murmur_hash(int* key) {
		unsigned h = (unsigned)*key;

		h *= 0xcc9e2d51;
		h = (h << 15) | (h >> 17);
		h *= 0x1b873593;
		h ^= 0;
		h = (h << 13) | (h >> 19);
		h = h * 5 + 0xe6546b64;
		h ^= 4;
		h ^= h >> 16;
		h *= 0x85ebca6b;
		h ^= h >> 13;
		h *= 0xc2b2ae35;
		h ^= h >> 16;

		return h;
	}
};

double relative_error(int expected, int got) {
	return abs(got - expected) / (double)expected;
}

int main() {
	std::random_device rd;
	std::mt19937 gen(rd());

	const int N = (int) 1e6;
	for (int k : {1, 10, 1000, 10000, N / 10, N, N * 10}) {
		std::uniform_int_distribution<> dis(1, k);
		std::set<int> all;
		UniqCounter counter;
		for (int i = 0; i < N; i++) {
			int value = dis(gen);
			all.insert(value);
			counter.add(value);
		}
		int expected = (int)all.size();
		int counter_result = counter.get_uniq_num();
		double error = relative_error(expected, counter_result);
		printf("%d numbers in range [1 .. %d], %d uniq, %d result, %.5f relative error\n", N, k, expected, counter_result, error);
		assert(error <= 0.1);
	}

	return 0;
}