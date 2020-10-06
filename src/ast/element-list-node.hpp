
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// ------------------------------------------------------------- ElementListNode
//
class ElementListNode final : public AstNode
{
 public:
   ElementListNode()
       : AstNode(NodeType::ELEMENTLIST)
   {}
   virtual ~ElementListNode() = default;

   ElementNode* element(size_t index) noexcept
   {
      return cast_child_<ElementNode>(index);
   }

   const ElementNode* element(size_t index) const noexcept
   {
      return cast_child_<const ElementNode>(index);
   }

   std::ostream& stream(std::ostream& ss,
                        string_view buffer) const noexcept override
   {
      for(size_t i = 0; i < size(); ++i) {
         if(i > 0) ss << " ";
         ss << PP{children()[i], buffer};
      }
      return ss;
   }
};

} // namespace giraffe
