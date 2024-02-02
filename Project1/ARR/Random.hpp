#include <random>

inline std::random_device device;
inline std::mt19937_64 RNGen(device());
inline std::uniform_real_distribution<float> myrandomf(0.0f, 1.0f);
inline std::uniform_real_distribution<double> myrandomd(0.0, 1.0);

inline int GetRandom(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(RNGen);
}