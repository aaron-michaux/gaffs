
#pragma once

#include "scanner/token.hpp"

namespace giraffe
{
// ---------------------------------------------------------- Predfine All Nodes

class AstNode;
class GrammarNode;
class RuleNode;
class ElementListNode;
class ElementNode;

// --------------------------------------------------------------------- AstNode

class AstNode
{
 protected:
   AstNode* parent_           = nullptr;
   vector<AstNode*> children_ = {};

 public:
   AstNode()               = default;
   AstNode(AstNode&&)      = default;
   AstNode(const AstNode&) = delete;
   virtual ~AstNode() { std::destroy(begin(children_), end(children_)); }
   AstNode& operator=(AstNode&&) = default;
   AstNode& operator=(const AstNode&) = delete;

   const AstNode* parent() const noexcept { return parent_; }
   const auto& children() const noexcept { return children_; }

   void set_children(vector<AstNode*>&& children) noexcept
   {
      children_ = std::move(children);
      for(auto ptr : children_) ptr->parent_ = this;
   }

   size_t size() const noexcept { return children_.size(); }
   bool empty() const noexcept { return children_.empty(); }

   virtual std::ostream& stream(std::ostream&) const noexcept = 0;
};

inline std::ostream& operator<<(std::ostream& os, AstNode* node)
{
   return node->stream(os);
}

} // namespace giraffe
