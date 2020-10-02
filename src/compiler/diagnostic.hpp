
#pragma once

#include "scanner/source-location.hpp"

namespace giraffe
{
struct Diagnostic final
{
   enum Level : uint8_t {
      NONE = 0,
      INFO,
      WARN,
      ERROR, // attempts to recover
      FATAL  // halts parsing
   };

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
};

} // namespace giraffe
