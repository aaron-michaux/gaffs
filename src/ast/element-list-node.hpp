
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// ------------------------------------------------------------- ElementListNode
//
class ElementListNode final : public AstNode
{
 public:
   virtual ~ElementListNode() = default;

   const ElementNode* element(size_t index) noexcept
   {
      assert(index < size());
      return reinterpret_cast<const ElementNode*>(children()[index]);
   }

   std::ostream& stream(std::ostream& ss) const noexcept override
   {
      for(size_t i = 0; i < size(); ++i) {
         if(i > 0) ss << " ";
         ss << children()[i];
      }
      return ss;
   }
};

} // namespace giraffe
