
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// ----------------------------------------------------------------- GrammarNode
//
class GrammarNode final : public AstNode
{
 private:
   unique_ptr<Scope> scope_ = nullptr; // Should be the global scope

 public:
   GrammarNode()
       : AstNode(NodeType::GRAMMAR)
   {}
   virtual ~GrammarNode() = default;

   Scope* scope() noexcept { return scope_.get(); }
   const Scope* scope() const noexcept { return scope_.get(); }
   void set_scope(unique_ptr<Scope>&& o) noexcept { scope_ = std::move(o); }

   RuleNode* rule(size_t index) noexcept
   {
      return cast_child_<RuleNode>(index);
   }

   const RuleNode* rule(size_t index) const noexcept
   {
      return cast_child_<const RuleNode>(index);
   }

   std::ostream& stream(std::ostream& ss,
                        string_view buffer) const noexcept override
   {
      for(const auto ptr : children()) ss << "\n" << PP{ptr, buffer};
      ss << "\n";
      return ss;
   }
};

} // namespace giraffe
