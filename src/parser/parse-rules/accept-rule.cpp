
#include "parser/parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------------- recovery

bool recover_to_next_rule(Scanner& tokens) noexcept
{
   return skip_to_sequence(tokens, recovery_seq_rule);
}

// ----------------------------------------------------------------- accept rule

static bool try_push_1_elem_list(CompilerContext& context,
                                 vector<AstNode*>& elem_lists)
{
   Scanner& tokens = context.scanner();
   while(true) {
      if(expect(tokens, first_set_element_list)) {
         elem_lists.push_back(accept_element_list(context));
         return true; // We got 1 element list

      } else {
         context.push_error("expected an `element list`"s);

         // Attempt to find next element-list
         if(skip_past_element(tokens)) continue; // try again

         recover_to_next_rule(tokens); // we failed, so go no next rule...
         return false;                 // we cannot get an element-list
      }
   }
}

// ----------------------------------------------------------------- accept rule

RuleNode* accept_rule(CompilerContext& context) noexcept
{
   Scanner& tokens = context.scanner();
   auto rule       = make_unique<RuleNode>();

   assert(expect(tokens, first_set_rule));

   rule->set_identifier(tokens.consume());
   assert(rule->identifier().id() == TIDENTIFIER);

   if(!accept(tokens, TCOLON)) {
      context.push_error("expected ':'"s);
      recover_to_next_rule(tokens);
      return rule.release();
   }

   vector<AstNode*> elem_lists;

   // Get the 1st element
   if(!try_push_1_elem_list(context, elem_lists)) return rule.release(); // bail

   while(true) {
      if(accept(tokens, TPIPE)) { // get subsequent element
         if(!try_push_1_elem_list(context, elem_lists))
            return rule.release(); // bail

      } else if(accept(tokens, TSEMICOLON)) { // normal termination
         break;                               // the rule ends normally

      } else { // okay, try to get another element anyway
         if(!try_push_1_elem_list(context, elem_lists))
            return rule.release(); // bail if impossible
      }
   }

   rule->set_children(std::move(elem_lists));

   return rule.release();
}

} // namespace giraffe
