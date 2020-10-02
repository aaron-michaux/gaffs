
#include "parser-internal.hpp"

namespace giraffe
{
unique_ptr<GrammarNode> parse(CompilerContext& context,
                              Scanner& tokens) noexcept
{
   if(!expect(tokens, first_set_grammar)) {
      push_error(context, tokens.current().location(), "expected start token");

      // recovery
      skip_to_sequence(tokens, TSTART);
   }

   return !expect(tokens, first_set_grammar)
              ? nullptr // we cannot parse the grammar!
              : unique_ptr<GrammarNode>(accept_grammar(context, tokens));
}
}; // namespace giraffe
