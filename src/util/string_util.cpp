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