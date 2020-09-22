#ifndef CHESS_AI_UTIL_MATH_UTIL_H_
#define CHESS_AI_UTIL_MATH_UTIL_H_

#include <type_traits>
#include <utility>
#include <random>


namespace math {

static std::mt19937 mersenne_twister_32bit_randomizer;
static std::uniform_real_distribution<double> rng_distribution{0.0, 1.0};

template<typename T>
inline constexpr
int signum(T x, std::false_type is_signed) {
  return T(0) < x;
}

template<typename T>
inline constexpr
int signum(T x, std::true_type is_signed) {
  return (T(0) < x) - (x < T(0));
}

template<typename T>
inline constexpr
int signum(T x) {
  return signum(x, std::is_signed<T>());
}

template<typename T>
inline constexpr
int sgn(T x) {
  return signum(x);
}

// [lower, upper)
double random(double lower, double upper);
// [0.0, upper)
double random(double upper);
// [0.0, 1.0)
double random();

// [lower, upper)
float random(float lower, float upper);
// [0.0f, upper)
float random(float upper);

// [min,max]
int random(int min, int max);
// [0, size-1]
int random(int size);

// clamp x to [range.first, range.second]
void clamp(double &x, std::pair<double, double> range);
void clamp(float &x, std::pair<float, float> range);

// true with probability p
bool chance(double p);

}

#endif // CHESS_AI_UTIL_MATH_UTIL_H_
