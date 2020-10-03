
#include "source-location.hpp"

namespace giraffe
{
std::pair<size_t, size_t> find_line(string_view text, size_t offset) noexcept
{
   assert(offset < text.size());

   // Search backwards to find either the start, or a newline character
   size_t line_start = offset;
   while(line_start != 0 && text[line_start - 1] != '\n') --line_start;

   size_t line_end = offset;
   while(line_end < text.size()) {
      const auto ch = text[line_end];
      if(ch == '\n' || ch == '\r') break;
      ++line_end;
   }

   return {line_start, line_end};
}

} // namespace giraffe
