

#include "parser-internal.hpp"

namespace giraffe
{
// -------------------------------------------------------------- accept element

ElementNode* accept_element(CompilerContext&, Scanner&) noexcept
{
   // First Set [ IDENTIFIER, STRING, LPARAN ]
   // Follow Set [ IDENTIFIER, STRING, LPARAN, PIPE, SEMICOLON ]
   return nullptr;
}

} // namespace giraffe
