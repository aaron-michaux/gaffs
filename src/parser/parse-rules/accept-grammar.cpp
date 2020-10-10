

#include "parser/parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------- accept grammar

GrammarNode* accept_grammar(CompilerContext& context) noexcept
{
   Scanner& tokens = context.scanner();
   auto grammar    = make_unique<GrammarNode>();

   assert(expect(tokens, first_set_grammar));
   if(!accept(tokens, TSTART)) assert(false);

   vector<AstNode*> rules;
   while(tokens.has_next()) {
      if(expect(tokens, TEOF)) {
         break; // We're done

      } else if(expect(tokens, first_set_rule)) { // i.e., TIDENTIFIER
         rules.push_back(accept_rule(context));

      } else { // Syntax error
         context.push_error("expected a new rule"s);

         // recovery
         if(!recover_to_next_rule(tokens)) break; // we got nothing
      }
   }

   grammar->set_children(std::move(rules));

   if(!expect(tokens, TEOF)) {
      TRACE(format("got token {}, pos = {}",
                   token_id_to_str(tokens.current().id()),
                   tokens.position()));
      context.push_error("expected end-of-file");
   }

   return grammar.release();
}

} // namespace giraffe
