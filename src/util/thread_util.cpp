#include "thread_util.h"

void thread::sleep_seconds(int seconds) { std::this_thread::sleep_for(std::chrono::seconds(seconds)); }
void thread::sleep_minutes(int minutes) { std::this_thread::sleep_for(std::chrono::minutes(minutes)); }
void thread::sleep_millis(int milli) { std::this_thread::sleep_for(std::chrono::milliseconds(milli)); }

void thread::sleep(int seconds) { sleep_seconds(seconds); }

void thread::wait_for(const bool &exit_condition) {
  do_while_waiting_for([] {}, exit_condition);
}

void thread::wait_for(const std::function<bool()> &exit_condition) {
  do_while_waiting_for([] {}, exit_condition);
}

void thread::wait_for_timeout(const bool &exit_condition, int timeout_seconds) {
  wait_for_timeout([&] { return exit_condition; }, timeout_seconds);
}

void thread::wait_for_timeout(const std::function<bool()> &exit_condition, int timeout_seconds) {
  int counter = timeout_seconds * 1000;
  do_while_waiting_for([] {}, [&] { return exit_condition() || counter-- < 0; });
}

void thread::do_while_waiting_for(const std::function<void()> &to_do, const bool &exit_condition) {
  do_while_waiting_for(to_do, [&] { return exit_condition; });
}
void thread::do_while_waiting_for(const std::function<void()> &to_do, const std::function<bool()> &exit_condition) {
  while (!exit_condition()) {
    to_do();
    sleep_millis(1);
  }
}