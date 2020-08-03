#ifndef CHESS_AI_UTIL_ASSERT_UTIL_H_
#define CHESS_AI_UTIL_ASSERT_UTIL_H_

#if (defined(DEBUG) && DEBUG == 1)
  #ifdef NDEBUG
    #undef NDEBUG
  #endif
  #include <cassert>

  #define fatal_assert() assert(false)  // macro to kill program during debug AND release
  #define debug_assert() assert(false)  // macro to kill program only during debug
// if DEBUG is defined -> Debug Mode

#else
  #define fatal_assert() { __builtin_unreachable(); exit(-1); }  // macro to kill program during debug AND release
  #define debug_assert()                                    // macro to kill program only during debug
// else DEBUG is not defined - Release/Testing/Production Mode

#endif
// end DEBUG if/else statement

#endif // CHESS_AI_UTIL_ASSERT_UTIL_H_
