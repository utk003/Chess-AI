#ifndef CHESS_AI_UTIL_UTIL_H
#define CHESS_AI_UTIL_UTIL_H

#include <thread>
#include <chrono>
#include <functional>

#include "math_util.h"
#include "string_util.h"

namespace thread {

void sleep_seconds(int seconds);
void sleep_minutes(int minutes);
void sleep_millis(int milli);

void sleep(int seconds);

void wait_for(const bool &exit_condition);
void wait_for(const std::function<bool()> &exit_condition);

void do_while_waiting_for(const std::function<void()> &to_do, const bool &exit_condition);
void do_while_waiting_for(const std::function<void()> &to_do, const std::function<bool()> &exit_condition);

template <class Fn, class... Args>
inline void create(Fn&& fn, Args&&... args) {
  std::thread(fn, args...).detach();
}

}

#endif // CHESS_AI_UTIL_UTIL_H
