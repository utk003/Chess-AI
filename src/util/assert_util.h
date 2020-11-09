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

#ifndef CHESS_AI_UTIL_ASSERT_UTIL_H_
#define CHESS_AI_UTIL_ASSERT_UTIL_H_

#ifdef DEBUG
#  include <iostream>
void printstacktracehelper(std::ostream &out, int depth);
#  define print_stack_trace() printstacktracehelper(std::cerr, 25) // debugging tool
#
#  undef NDEBUG // allow assert()
#  include <cassert>
#  define FATAL_ASSERT { assert(false); } // macro to kill program during debug AND release
#  define DEBUG_ASSERT { printstacktracehelper(std::cerr, 25); assert(false); } // macro to kill program only during debug
// if DEBUG is defined -> Debug Mode

#else
#  define print_stack_trace() // debugging tool
#  define FATAL_ASSERT { __builtin_unreachable(); exit(-1); } // macro to kill program during debug AND release
#  define DEBUG_ASSERT {}                                     // macro to kill program only during debug
// else DEBUG is not defined -> Release/Testing/Production Mode

#endif
// end DEBUG if/else statement

#endif // CHESS_AI_UTIL_ASSERT_UTIL_H_
