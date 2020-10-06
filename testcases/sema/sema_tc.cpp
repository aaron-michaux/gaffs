
#define CATCH_CONFIG_PREFIX_ALL

#include "catch.hpp"

#include "stdinc.hpp"

#include "compiler/compiler-context.hpp"
#include "parser/parser.hpp"
#include "scanner/scanner.hpp"
#include "sema/sema.hpp"

namespace giraffe
{
static constexpr string_view test_str_sema_0 = R"V0G0N(
// Tokens are "ALL-CAPs", or strings

Grammar: Rule* EOF ;

Rule: IDENTIFIER ':' ElementList ('|' ElementList)* ';' ;

ElementList: (Element ElementSuffix?)+ ;

Element: IDENTIFIER | STRING | '(' ElementList ')' ;

ElementSuffix: '*' | '+' | '?' ;

)V0G0N";

CATCH_TEST_CASE("Sema", "[sema]")
{
   // This code should just finish without tripping the memory sanitizer
   CATCH_SECTION("sema")
   { //
      TRACE("Hello Sema!");

      if(false) {
         Scanner tokens(test_str_sema_0);
         while(tokens.has_next())
            cout << tokens.consume().to_string(tokens.current_buffer()) << endl;
      }

      CompilerContext context = make_compiler_context(test_str_sema_0);
      auto ast                = parse(context);
      auto global_scope       = build_symbol_table(context, ast.get());

      calculate_first_final_follow_sets(context, global_scope.get(), ast.get());

      ast->stream(cout, context.tokens.current_buffer());
      context.stream(cout);
      cout << '\n';
      global_scope->stream(cout);
   }
}

} // namespace giraffe
