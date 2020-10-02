
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// ----------------------------------------------------------------- GrammarNode
//
class GrammarNode final : public AstNode
{
 public:
   virtual ~GrammarNode() = default;

   const RuleNode* rule(size_t index) const noexcept
   {
      assert(index < size());
      return reinterpret_cast<const RuleNode*>(children()[index]);
   }

   std::ostream& stream(std::ostream& ss) const noexcept override
   {
      for(const auto ptr : children()) ss << "\n" << ptr;
      ss << "\n";
      return ss;
   }
};

} // namespace giraffe
