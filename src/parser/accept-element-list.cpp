

#include "parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------------- recovery

bool recover_to_next_element_list(TokenProducer& tokens) noexcept
{
   return skip_to_sequence(tokens, first_set_element_list);
}

// --------------------------------------------------------- accept element list

ElementListNode* accept_element_list(CompilerContext&, TokenProducer&) noexcept
{
   // First Set [ IDENTIFIER, STRING, LPARAN ]
   // Follow Set [ PIPE, SEMICOLON ]
   return nullptr;
}

} // namespace giraffe
