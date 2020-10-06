
#pragma once

#include "scanner/scanner.hpp"
#include "utils/in-list.hpp"

namespace giraffe::detail
{
template<typename T> bool match_worker(Scanner& tokens, T&& id) noexcept
{
   if(in_list(tokens.current().id(), id)) {
      tokens.consume();
      return true;
   }
   return false;
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

template<typename O, typename... Ts>
bool skip_to_sequence_omitting(Scanner& tokens,
                               const O& omit,
                               Ts&&... ids) noexcept
{
   while(tokens.has_next() && !in_list(tokens.current().id(), omit)) {
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
