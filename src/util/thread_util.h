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
