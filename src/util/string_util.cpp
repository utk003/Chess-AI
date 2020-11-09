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

#include "string_util.h"

#include <sstream>
#include <iomanip>
#include <limits>

std::string string::from_bool(bool b) {
  return b ? "true": "false";
}

bool string::to_bool(const std::string &s) {
  return s == "true";
}

std::string string::from_double(double d) {
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<double>::max_digits10) << std::scientific << d;
  return ss.str();
}

double string::to_double(const std::string &s) {
  double d;

  std::stringstream ss;
  ss << s;
  ss >> d;

  return d;
}

std::string string::combine(const std::vector<std::string> &strings, bool space_separated) {
  if (strings.empty())
    return "";

  std::string str;
  if (space_separated) {
    for (const auto &s: strings) {
      str += " ";
      str += s;
    }
    str = str.substr(1);
  } else
    for (const auto &s: strings)
      str += s;

  return str;
}