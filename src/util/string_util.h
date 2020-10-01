#ifndef CHESS_AI_UTIL_STRING_UTIL_H_
#define CHESS_AI_UTIL_STRING_UTIL_H_

#include <string>
#include <vector>

namespace string {

std::string from_bool(bool b);
bool to_bool(const std::string &s);

std::string from_double(double d);
double to_double(const std::string &s);

std::string combine(const std::vector<std::string> &strings, bool space_separated = false);

inline bool startsWith(const std::string &str, const std::string &start) {
  return str.substr(0, start.length()) == start;
}
inline bool endsWith(const std::string &str, const std::string &end) {
  return str.substr(str.length() - end.length()) == end;
}

}

#endif // CHESS_AI_UTIL_STRING_UTIL_H_
