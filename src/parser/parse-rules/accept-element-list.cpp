
#include "parser/parser-internal.hpp"

namespace giraffe
{
// --------------------------------------------------------- accept element list

ElementListNode* accept_element_list(CompilerContext& context) noexcept
{
   Scanner& tokens = context.scanner();
   assert(expect(tokens, first_set_element_list));

   auto elem_list = make_unique<ElementListNode>();

   vector<AstNode*> elems;
   while(tokens.has_next()) {
      if(expect(tokens, first_set_element)) {
         elems.push_back(accept_element(context));

      } else if(expect(tokens, follow_set_element_list)) {
         break; // we're done

      } else {
         context.push_error("expected `rule element`"s);
         if(skip_past_element(tokens) && expect(tokens, first_set_element))
            continue; // try again
         break;
      }
   }

   elem_list->set_children(std::move(elems));

   return elem_list.release();
}

} // namespace giraffe
