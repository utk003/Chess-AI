#include "math_util.h"

#include <random>
#include <string>
#include <chrono>
#include <ctime>

double run_randomizer() {
  static std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
  static time_t time = std::chrono::system_clock::to_time_t(time_on_start);
  static std::string string_time(ctime(&time));
  static std::seed_seq seed(string_time.begin(), string_time.end());
  static std::mt19937 randomizer_32bit{seed};
  static std::uniform_real_distribution<double> distribution{0.0, 1.0};

  return distribution(randomizer_32bit);
}

// [lower, upper)
double math::random(double lower, double upper) {
  return (upper - lower) * run_randomizer() + lower;
}
// [0.0, upper)
double math::random(double upper) { return random(0.0, upper); }
// [0.0, 1.0)
double math::random() { return random(0.0, 1.0); }

// [lower, upper)
float math::random(float lower, float upper) {
  return (float) random((double) lower, (double) upper);
}
// [0.0f, upper)
float math::random(float upper) { return random(0.0f, upper); }

// [min,max]
int math::random(int min, int max) {
  return (int) random((double) min, (double) max + 1.0);
}
// [0, size-1]
int math::random(int size) { return random(0, size-1); }

// true with probability prob
bool math::chance(double prob) {
  prob = prob > 1.0 ? 1.0: prob < 0.0 ? 0.0: prob; // limit prob to between 0 and 1
  return random() < prob;
}