#ifndef CHESS_AI_UTIL_THREAD_UTIL_H_
#define CHESS_AI_UTIL_THREAD_UTIL_H_

#include <thread>
#include <chrono>
#include <functional>

namespace thread {

void sleep_seconds(int seconds);
void sleep_minutes(int minutes);
void sleep_millis(int milli);

void sleep(int seconds);

void wait_for(const bool &exit_condition);
void wait_for(const std::function<bool()> &exit_condition);

void wait_for_timeout(const bool &exit_condition, int timeout_seconds);
void wait_for_timeout(const std::function<bool()> &exit_condition, int timeout_seconds);

void do_while_waiting_for(const std::function<void()> &to_do, const bool &exit_condition);
void do_while_waiting_for(const std::function<void()> &to_do, const std::function<bool()> &exit_condition);

template<class Fn, class... Args>
inline void create(Fn &&fn, Args &&... args) {
  std::thread(fn, args...).detach();
}

}

#endif // CHESS_AI_UTIL_THREAD_UTIL_H_
