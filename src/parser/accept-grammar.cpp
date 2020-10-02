
#include "parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------- accept grammar

GrammarNode* accept_grammar(CompilerContext& context,
                            Scanner& tokens) noexcept
{
   auto grammar = make_unique<GrammarNode>();

   assert(expect(tokens, first_set_grammar));

   vector<AstNode*> rules;
   while(tokens.has_next()) {
      if(expect(tokens, first_set_rule)) { // i.e., TIDENTIFIER
         rules.push_back(accept_rule(context, tokens));

      } else if(!expect(tokens, follow_set_rule)) { // Syntax error
         push_error(
             context, tokens.current().location(), "expected a new rule"s);
         // recovery
         if(!recover_to_next_rule(tokens)) break; // we got nothing
      }
   }

   grammar->set_children(std::move(rules));

   if(!expect(tokens, TEOF)) {
      push_error(context, tokens.current().location(), "expected end-of-file");
   }

   return grammar.release();
}

} // namespace giraffe
