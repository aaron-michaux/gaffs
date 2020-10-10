
#define This Diagnostic

#include "compiler-context.hpp"
#include "diagnostic.hpp"
#include "utils/in-list.hpp"

namespace giraffe
{
std::ostream& This::stream(std::ostream& ss,
                           const CompilerContext& context) const noexcept
{
   static constexpr auto sentence_ends = to_array<char>({'.', '!', '?'});

   const bool use_color = context.compiler_opts().color_diagnostics;

   ss << context.names().at(location.key);
   ss << ": ";
   if(use_color) { ss << k_level_colors.at(size_t(level)); }
   ss << k_level_names.at(size_t(level)) << ':';
   if(use_color) { ss << k_color_none; }
   ss << ' ' << message;
   if(message.size() > 0 && !in_list(message.back(), sentence_ends)) ss << '.';
   ss << '\n';

   const auto text                   = context.texts().at(location.key);
   const auto [line_start, line_end] = find_line(text, location.offset);
   assert(location.offset >= line_start);
   assert(location.offset <= line_end);
   assert(line_start < text.size());
   assert(line_end <= text.size());
   if(line_end > line_start) { // the string is non-empty
      // The line of code
      ss << string_view(&text[line_start], line_end - line_start) << '\n';

      // The carrot
      for(size_t i = line_start; i < location.offset; ++i) ss << ' ';
      if(use_color) { ss << k_light_green; }
      ss << '^';
      if(use_color) { ss << k_color_none; }
      ss << '\n';
   }

   return ss;
}

} // namespace giraffe

#undef This
