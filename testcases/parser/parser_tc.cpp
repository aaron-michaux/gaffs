
#define CATCH_CONFIG_PREFIX_ALL

#include "catch.hpp"

#include "stdinc.hpp"

#include "driver/compiler-context.hpp"
#include "parser/parser.hpp"
#include "scanner/scanner.hpp"
#include "sema/sema.hpp"

namespace giraffe
{
CATCH_TEST_CASE("Parser", "[parser]")
{
   // This code should just finish without tripping the memory sanitizer
   CATCH_SECTION("parser")
   { //
   }
}

} // namespace giraffe
