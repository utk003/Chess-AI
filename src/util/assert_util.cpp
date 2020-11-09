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

#ifdef DEBUG

#include "assert_util.h"

#ifndef _WIN32
#  include <execinfo.h>
#  include <iostream>
#  include <cxxabi.h>
#endif

void printstacktracehelper(std::ostream &out, int depth) {
#ifndef _WIN32
  // trace header
  out << "Stack Trace:";

  // storage array for stack trace address data
  void *addr_list[depth];

  // retrieve current stack addresses
  int addr_len = backtrace(addr_list, depth);

  // exit if no trace found
  if (addr_len == 0) {
    out << " EMPTY" << " -> " << "POSSIBLY CORRUPT" << std::endl;
    return;
  }
  out << std::endl; // after check for empty trace

  // get trace into string form (mangled)
  char **symbol_list = backtrace_symbols(addr_list, addr_len);

  // allocate string which will be filled with the demangled function name
  size_t func_name_size = 256;
  char *func_name = (char *) malloc(func_name_size);

  // skip i = 0 b/c it's this function!
  for (int i = 1; i < addr_len; i++) {
    // parse non-OSX traces
    char *begin_name = nullptr;
    char *end_name = nullptr;
    char *begin_offset = nullptr;
    char *end_offset = nullptr;

    for (char *p = symbol_list[i]; *p; ++p) {
      if (*p == '(')
        begin_name = p;
      else if (*p == '+')
        begin_offset = p;
      else if (*p == ')' && begin_offset) {
        end_offset = p;
        break;
      }
    }

    // OSX edge-case -> Courtesy of Ben Haller
    // https://github.com/MesserLab/SLiM/blob/master/eidos/eidos_globals.cpp
    if (!(begin_name && begin_offset && end_offset && begin_name < begin_offset)) {
      // define ParseState enum for parsing help
      enum class ParseState {
          kInWhitespace1 = 1,
          kInLineNumber,
          kInWhitespace2,
          kInPackageName,
          kInWhitespace3,
          kInAddress,
          kInWhitespace4,
          kInFunction,
          kInWhitespace5,
          kInPlus,
          kInWhitespace6,
          kInOffset,
          kInOverrun
      };
      ParseState parse_state = ParseState::kInWhitespace1;

      char *p; // declared outside for end_offset value
      for (p = symbol_list[i]; *p; ++p)
        switch (parse_state) {
          case ParseState::kInWhitespace1:
            if (!isspace(*p))
              parse_state = ParseState::kInLineNumber;
            break;

          case ParseState::kInLineNumber:
            if (isspace(*p))
              parse_state = ParseState::kInWhitespace2;
            break;

          case ParseState::kInWhitespace2:
            if (!isspace(*p))
              parse_state = ParseState::kInPackageName;
            break;

          case ParseState::kInPackageName:
            if (isspace(*p))
              parse_state = ParseState::kInWhitespace3;
            break;

          case ParseState::kInWhitespace3:
            if (!isspace(*p))
              parse_state = ParseState::kInAddress;
            break;

          case ParseState::kInAddress:
            if (isspace(*p))
              parse_state = ParseState::kInWhitespace4;
            break;

          case ParseState::kInWhitespace4:
            if (!isspace(*p)) {
              parse_state = ParseState::kInFunction;
              begin_name = p - 1;
            }
            break;

          case ParseState::kInFunction:
            if (isspace(*p)) {
              parse_state = ParseState::kInWhitespace5;
              end_name = p;
            }
            break;

          case ParseState::kInWhitespace5:
            if (!isspace(*p))
              parse_state = ParseState::kInPlus;
            break;

          case ParseState::kInPlus:
            if (isspace(*p))
              parse_state = ParseState::kInWhitespace6;
            break;

          case ParseState::kInWhitespace6:
            if (!isspace(*p)) {
              parse_state = ParseState::kInOffset;
              begin_offset = p - 1;
            }
            break;

          case ParseState::kInOffset:
            if (isspace(*p)) {
              parse_state = ParseState::kInOverrun;
              end_offset = p;
            }
            break;

          case ParseState::kInOverrun:
            break;

          default:
            std::cerr << "Stack Trace Finder - Recursive Failure!!" << " @ "
                      << __FILE__ << " " << __LINE__ << std::endl;
            FATAL_ASSERT
        }

      if (parse_state == ParseState::kInOffset && !end_offset)
        end_offset = p;
    }

    if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
      *begin_name++ = '\0';
      if (end_name)
        *end_name = '\0';
      *begin_offset++ = '\0';
      *end_offset = '\0';

      int status;
      char *ret = abi::__cxa_demangle(begin_name, func_name, &func_name_size, &status);

      // demangled fine
      if (status == 0) {
        func_name = ret; // use possibly realloc()-ed string; static analyzer doesn't like this but it is OK I think
        out << symbol_list[i] << " : " << func_name << " + " << begin_offset << " " << std::endl;
      } else // something went wrong
        out << symbol_list[i] << " : " << begin_name << "() + " << begin_offset << " " << std::endl;
    } else
      out << symbol_list[i] << std::endl;
  }

  free(func_name);
  free(symbol_list);
#endif
}
#endif