
#define CATCH_CONFIG_PREFIX_ALL

#include "catch.hpp"

#include "stdinc.hpp"

#include "scanner/token-producer.hpp"

namespace giraffe
{
static constexpr string_view scanner_test_str_0 = R"V0G0N(
// Tokens are "ALL-CAPs", or strings

Grammar: Rule* EOF ;

Rule: IDENTIFIER ':' Alternatives ';' ;

Alternatives: Alternative ('|' Alternative)* ;

Alternative: (Element ElementSuffix?)+ ;

Element: IDENTIFIER | STRING | '(' Alternatives ')' ;

ElementSuffix: '*' | '+' | '?' ;
)V0G0N";

CATCH_TEST_CASE("Scanner", "[scanner]")
{
   // This code should just finish without tripping the memory sanitizer
   CATCH_SECTION("scanner")
   { //
      TokenProducer tokens(scanner_test_str_0);

      //// while(tokens.has_next()) cout << tokens.consume().to_string() <<
      /// endl;
   }
}

} // namespace giraffe
