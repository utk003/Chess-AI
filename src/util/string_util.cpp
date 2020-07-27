#include "string_util.h"

std::string string::string(bool b) {
  return b ? "true": "false";
}

bool string::boolean(const std::string &s) {
  return s == "true";
}