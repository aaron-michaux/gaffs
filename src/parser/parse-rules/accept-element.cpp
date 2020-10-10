
#include "parser/parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------------- recovery

bool skip_past_element(Scanner& tokens) noexcept
{
   return skip_to_sequence(tokens, follow_set_element);
}

// -------------------------------------------------------------- accept element

ElementNode* accept_element(CompilerContext& context) noexcept
{
   Scanner& tokens = context.scanner();
   assert(expect(tokens, first_set_element));

   auto elem = make_unique<ElementNode>();

   const auto id = tokens.current().id();
   elem->set_token(tokens.consume());

   if(id == TLPAREN) { // an element list
      constexpr auto recovery_set = to_array<int>({TSEMICOLON, TRPAREN});
      auto attempt_recovery       = [&]() {
         skip_to_sequence(tokens, recovery_set);
         if(tokens.current().id() == TRPAREN) {
            tokens.consume();
            return true; // we survived
         }
         return false; // bail
      };

      if(expect(tokens, first_set_element_list)) {
         elem->set_children({accept_element_list(context)});
         if(!accept(tokens, TRPAREN)) {
            if(!attempt_recovery()) return elem.release(); // outta here
         }
      } else {
         elem->set_children({new ElementListNode});
         context.push_error("expected an `element list`"s);
         if(!attempt_recovery()) return elem.release(); // outta here
      }
   }

   if(in_list(tokens.current().id(), token_suffix_list))
      elem->set_suffix(tokens.consume());

   return elem.release();
}

} // namespace giraffe
