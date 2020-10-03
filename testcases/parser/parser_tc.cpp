
#define CATCH_CONFIG_PREFIX_ALL

#include "catch.hpp"

#include "stdinc.hpp"

#include "compiler/compiler-context.hpp"
#include "parser/parser.hpp"
#include "scanner/scanner.hpp"

namespace giraffe
{
static constexpr string_view test_str_0 = R"V0G0N(
// Tokens are "ALL-CAPs", or strings

Grammar: Rule* EOF ;

Rule: IDENTIFIER ':' ElementList ('|' ElementList)* ';' ;

ElementList: (Element ElementSuffix?)+ ;

Element: IDENTIFIER | STRING | '(' ElementList ')' ;

ElementSuffix: '*' | '+' | '?' ;
)V0G0N";

CATCH_TEST_CASE("Parser", "[parser]")
{
   // This code should just finish without tripping the memory sanitizer
   CATCH_SECTION("parser")
   { //
      TRACE("Hello parser!");

      if(false) {
         Scanner tokens(test_str_0);
         while(tokens.has_next())
            cout << tokens.consume().to_string(tokens.current_buffer()) << endl;
      }

      CompilerContext context = make_compiler_context(test_str_0);
      auto ast                = parse(context);
      ast->stream(cout, context.tokens.current_buffer());

      context.stream(cout);
   }
}

} // namespace giraffe
