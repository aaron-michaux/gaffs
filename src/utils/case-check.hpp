
#pragma once

namespace giraffe
{
inline bool is_uppercase_string(string_view s) noexcept
{
   return std::all_of(
       cbegin(s), cend(s), [](auto c) { return std::isupper(c); });
}

inline bool is_justified_string(string_view s) noexcept
{
   if(s.empty()) return true;
   return std::isupper(s.front())
          && std::any_of(std::next(cbegin(s)), cend(s), [](auto c) {
                return !std::isupper(c);
             });
}

} // namespace giraffe
