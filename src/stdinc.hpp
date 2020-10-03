
#ifndef GAFFS__STDINC_HPP__INCLUDE_GUARD__
#define GAFFS__STDINC_HPP__INCLUDE_GUARD__

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "fmt/format.h"

#include "utils/to-array.hpp"

namespace giraffe
{
using namespace std::string_literals;

using std::array;
using std::make_unique;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

using fmt::format;
using std::function;

using std::cout;
using std::endl;

using std::begin;
using std::cbegin;
using std::cend;
using std::end;

#ifdef NDEBUG
#define TRACE(msg)
#else
#define TRACE(msg)                                                             \
   {                                                                           \
      std::cout << format(                                                     \
          "\x1b[97m\x1b[42mTRACE\x1b[0m {}:{}: {}", __FILE__, __LINE__, (msg)) \
                << std::endl;                                                  \
   }
#endif

#define FATAL(msg)                                                             \
   {                                                                           \
      std::cerr << format(                                                     \
          "\x1b[97m\x1b[41mFATAL\x1b[0m {}:{}: {}", __FILE__, __LINE__, (msg)) \
                << std::endl;                                                  \
      exit(1);                                                                 \
   }

} // namespace giraffe

#endif
