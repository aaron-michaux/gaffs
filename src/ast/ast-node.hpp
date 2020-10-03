
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

// ---------------------------------------------------------------------- Detail
// to assist in printing
using AstNodeSV = std::pair<const AstNode*, string_view>;

// --------------------------------------------------------------------- AstNode

class AstNode
{
 protected:
   AstNode* parent_           = nullptr;
   vector<AstNode*> children_ = {};

   using PP = AstNodeSV;

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

   virtual std::ostream& stream(std::ostream&, string_view) const noexcept = 0;
};

inline std::ostream& operator<<(std::ostream& ss, AstNodeSV pp)
{
   if(pp.first == nullptr)
      ss << "<nullptr>";
   else
      pp.first->stream(ss, pp.second);
   return ss;
}

} // namespace giraffe
