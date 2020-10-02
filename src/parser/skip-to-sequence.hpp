
#pragma once

#include "scanner/scanner.hpp"

#include <type_traits>

namespace giraffe::detail
{
template<typename T> bool match_worker(Scanner& tokens, T&& id) noexcept
{
   if constexpr(std::is_integral<std::remove_reference_t<T>>::value) {
      if(int(id) == tokens.current().id()) {
         tokens.consume(); // advance on space
         return true;      // we're winners!
      } else {
         return false;
      }
   } else {
      if(std::any_of(cbegin(id), cend(id), [&](auto&& x) {
            return match_worker(tokens, x);
         })) {
         tokens.consume();
         return true;
      }
      return false;
   }
}

template<typename T, typename... Ts>
bool match_worker(Scanner& tokens, T&& id, Ts&&... rest) noexcept
{
   return match_worker(tokens, id)
          && match_worker(tokens, std::forward<Ts>(rest)...);
}
} // namespace giraffe::detail

namespace giraffe
{
template<typename... Ts>
bool skip_to_sequence(Scanner& tokens, Ts&&... ids) noexcept
{
   while(tokens.has_next()) {
      const auto start_position = tokens.position();
      const bool match = detail::match_worker(tokens, std::forward<Ts>(ids)...);
      tokens.set_position(start_position);
      if(match)
         return true;
      else
         tokens.consume(); // advance one token
   }
   return false;
}

} // namespace giraffe
