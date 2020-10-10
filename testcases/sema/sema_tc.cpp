
#define CATCH_CONFIG_PREFIX_ALL

#include "catch.hpp"

#include "stdinc.hpp"

#include "driver/compiler-context.hpp"
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
   {
      auto context = CompilerContext::make(test_str_sema_0, {});
      assert(context != nullptr);

      auto ast = parse(*context);

      // Calculculate the global scope
      ast->set_scope(build_symbol_table(*context, ast.get()));

      // Finst/final/follow sets
      calculate_first_final_follow_sets(*context, ast.get());

      // LL (i.e., parse) rules
      calculate_ll_rules(*context, ast.get());

      if(false) { // Output some nice friendly stuff
         cout << '\n';
         ast->scope()->stream(cout);
         ast->stream(cout, context->curried_text_function());

         for(AstNode* r_node : *ast) {
            auto rule      = cast_ast_node<RuleNode>(r_node);
            auto print_set = [&](auto& set) {
               return implode(cbegin(set), cend(set), ", ");
            };
            cout << format("Rule: {}\n", text(*context, rule->identifier()));
            cout << format("   first Set:  {}\n", print_set(rule->first_set()));
            cout << format("   final Set:  {}\n", print_set(rule->final_set()));
            cout << format("   follow Set: {}\n",
                           print_set(rule->follow_set()));
            cout << endl;
         }
      }
   }
}

} // namespace giraffe
