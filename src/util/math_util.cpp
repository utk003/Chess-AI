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

#include "math_util.h"

#include <iostream>

#include "assert_util.h"

// [lower, upper)
double math::random(double lower, double upper) {
  return (upper - lower) * rng_distribution(mersenne_twister_32bit_randomizer) + lower;
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
int math::random(int size) { return random(0, size - 1); }

// clamp x to [range.first, range.second]
void math::clamp(double &x, std::pair<double, double> range) {
  if (range.first > range.second) {
    DEBUG_ASSERT
    double temp = range.first;
    range.first = range.second;
    range.second = temp;
  }

  if (x < range.first)
    x = range.first;
  else if (x > range.second)
    x = range.second;
}
void math::clamp(float &x, std::pair<float, float> range) {
  if (range.first > range.second) {
    DEBUG_ASSERT
    double temp = range.first;
    range.first = range.second;
    range.second = temp;
  }

  if (x < range.first)
    x = range.first;
  else if (x > range.second)
    x = range.second;
}

// true with probability p
bool math::chance(double p) {
  return random() < p; // don't need to explicitly clamp p to [0.0, 1.0] b/c this will still work
}