// ------------------------------------------------------------------------------ //
// MIT License                                                                    //
//                                                                                //
// Copyright (c) 2020 Utkarsh Priyam                                              //
//                                                                                //
// Permission is hereby granted, free of charge, to any person obtaining a copy   //
// of this software and associated documentation files (the "Software"), to deal  //
// in the Software without restriction, including without limitation the rights   //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      //
// copies of the Software, and to permit persons to whom the Software is          //
// furnished to do so, subject to the following conditions:                       //
//                                                                                //
// The above copyright notice and this permission notice shall be included in all //
// copies or substantial portions of the Software.                                //
//                                                                                //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  //
// SOFTWARE.                                                                      //
// ------------------------------------------------------------------------------ //

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
