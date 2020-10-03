
#pragma once

#include "scanner/source-location.hpp"

namespace giraffe
{
struct CompilerContext;

struct Diagnostic final
{
   enum Level : uint8_t {
      NONE = 0,
      INFO,
      WARN,
      ERROR, // attempts to recover
      FATAL  // halts parsing
   };

   static constexpr auto k_color_none   = string_view("\x1b[0m");
   static constexpr auto k_light_green  = string_view("\x1b[92m");
   static constexpr auto k_level_colors = to_array<string_view>(
       {"\x1b[0m", "\x1b[34m", "\x1b[93m", "\x1b[91m", "\x1b[41m\x1b[97m"});
   static constexpr auto k_level_names
       = to_array<string_view>({"none", "info", "warning", "error", "fatal"});

   string message          = ""s;
   SourceLocation location = {};
   uint32_t length         = 0; // from `loc`
   uint32_t carrot         = 0; // The highlight point
   Level level             = NONE;

   Diagnostic()                  = default;
   Diagnostic(const Diagnostic&) = default;
   Diagnostic(Diagnostic&&)      = default;
   ~Diagnostic()                 = default;
   Diagnostic& operator=(const Diagnostic&) = default;
   Diagnostic& operator=(Diagnostic&&) = default;

   Diagnostic(Level lv, SourceLocation loc, string&& msg) noexcept
       : message(std::move(msg))
       , location(loc)
       , level(lv)
   {}

   std::ostream& stream(std::ostream&, const CompilerContext&) const noexcept;
};

} // namespace giraffe
