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