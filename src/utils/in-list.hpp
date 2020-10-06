
#pragma once

#include <type_traits>

namespace giraffe
{
template<typename T> bool in_list(int id, T&& list) noexcept
{
   if constexpr(std::is_integral<std::remove_reference_t<T>>::value) {
      return id == int(list);
   } else {
      return std::any_of(
          cbegin(list), cend(list), [&](auto&& x) { return in_list(id, x); });
   }
}
} // namespace giraffe
