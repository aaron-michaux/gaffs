

#include "parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------------- recovery

bool recover_to_next_rule(Scanner& tokens) noexcept
{
   return skip_to_sequence(tokens, TIDENTIFIER, TCOLON, first_set_element_list);
}

// ----------------------------------------------------------------- accept rule

RuleNode* accept_rule(CompilerContext& context, Scanner& tokens) noexcept
{
   auto rule = make_unique<RuleNode>();

   assert(expect(tokens, first_set_rule));

   rule->set_identifier(tokens.consume());
   assert(rule->identifier().id() == TIDENTIFIER);

   if(!expect(tokens, TCOLON)) {
      push_error(context, tokens.current().location(), "expected ':'"s);
      recover_to_next_rule(tokens);
      return rule.release();
   }

   vector<AstNode*> elem_lists;

   if(!expect(tokens, first_set_element_list)) {
      push_error(
          context, tokens.current().location(), "expected a 'rule element'"s);
      if(!recover_to_next_element_list(tokens)) { // try to find the next list
         recover_to_next_rule(tokens); // we failed, so go no next rule...
         return rule.release();
      }
   }
   // elem_lists.push_back(accept_element_list(context, tokens));

   // while(tokens.has_next()) {

   // }

   // while(tokens.peek().id() == TPIPE) {
   //    expect(tokens, TPIPE);
   //    elem_lists.push_back(accept_element_list(context, tokens));
   // }
   // rule->set_children(std::move(elem_lists));

   if(!expect(tokens, TSEMICOLON)) {
      push_error(context,
                 tokens.current().location(),
                 "expected ';' at the end of a rule"s);
      recover_to_next_rule(tokens);
      return rule.release();
   }

   return rule.release();
}

} // namespace giraffe
