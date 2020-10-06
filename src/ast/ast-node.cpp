
#include "ast-node.hpp"

namespace giraffe
{
const char* str(NodeType o) noexcept
{
#define CASE(x) \
   case NodeType::x: return #x
   switch(o) {
      CASE(NONE);
      CASE(GRAMMAR);
      CASE(RULE);
      CASE(ELEMENTLIST);
      CASE(ELEMENT);
   }
#undef CASE
   assert(false);
   return "<unknown>";
}

} // namespace giraffe
