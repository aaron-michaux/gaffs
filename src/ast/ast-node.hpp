
#pragma once

#include "scanner/token.hpp"
#include "sema/symbol-table.hpp"

namespace giraffe
{
// ---------------------------------------------------------- Predfine All Nodes

class AstNode;
class GrammarNode;
class RuleNode;
class ElementListNode;
class ElementNode;

// --------------------------------------------------------------- NodeType Enum

enum class NodeType : uint8_t { NONE = 0, GRAMMAR, RULE, ELEMENTLIST, ELEMENT };

const char* str(NodeType) noexcept;

// ---------------------------------------------------------------------- Detail
// to assist in printing
namespace detail
{
   using AstNodeSV = std::pair<const AstNode*, string_view>;
}

// --------------------------------------------------------------------- AstNode

class AstNode
{
 private:
   AstNode* parent_           = nullptr;
   vector<AstNode*> children_ = {};
   size_t index_in_parent_    = 0;
   NodeType type_             = NodeType::NONE;

 protected:
   using PP = detail::AstNodeSV;

   template<typename T> T* cast_child_(size_t index) const noexcept
   {
      assert(index < children_.size());
      return reinterpret_cast<T*>(children_[index]);
   }

 public:
   AstNode(NodeType type)
       : type_(type)
   {}
   AstNode(AstNode&&)      = default;
   AstNode(const AstNode&) = delete;
   virtual ~AstNode()
   {
      std::destroy(std::begin(children_), std::end(children_));
   }
   AstNode& operator=(AstNode&&) = default;
   AstNode& operator=(const AstNode&) = delete;

   auto parent() const noexcept { return parent_; }
   const auto& children() const noexcept { return children_; }
   auto index_in_parent() const noexcept { return index_in_parent_; }

   auto begin() const noexcept { return children_.begin(); }
   auto rbegin() const noexcept { return children_.rbegin(); }
   auto end() const noexcept { return children_.end(); }
   auto rend() const noexcept { return children_.rend(); }

   auto front() const noexcept { return children_.front(); }
   auto back() const noexcept { return children_.back(); }

   void set_children(vector<AstNode*>&& children) noexcept
   {
      children_    = std::move(children);
      size_t index = 0;
      for(auto ptr : children_) {
         ptr->parent_          = this;
         ptr->index_in_parent_ = index++;
      }
   }

   NodeType type() const noexcept { return type_; }
   size_t size() const noexcept { return children_.size(); }
   bool empty() const noexcept { return children_.empty(); }

   virtual std::ostream& stream(std::ostream&, string_view) const noexcept = 0;
};

// ------------------------------------------------------------------ operator<<

inline std::ostream& operator<<(std::ostream& ss, detail::AstNodeSV pp)
{
   if(pp.first == nullptr)
      ss << "<nullptr>";
   else
      pp.first->stream(ss, pp.second);
   return ss;
}

// -------------------------------------------------- safely downcast an AstNode

template<std::derived_from<AstNode> T> T* cast_ast_node(AstNode* o) noexcept
{
   T* ret = nullptr;
   if constexpr(std::is_same_v<std::decay_t<T>, AstNode>) {
      ret = o; // not a cast
   } else if constexpr(std::is_same_v<std::decay_t<T>, GrammarNode>) {
      if(o->type() == NodeType::GRAMMAR) ret = reinterpret_cast<T*>(o);
   } else if constexpr(std::is_same_v<std::decay_t<T>, RuleNode>) {
      if(o->type() == NodeType::RULE) ret = reinterpret_cast<T*>(o);
   } else if constexpr(std::is_same_v<std::decay_t<T>, ElementListNode>) {
      if(o->type() == NodeType::ELEMENTLIST) ret = reinterpret_cast<T*>(o);
   } else if constexpr(std::is_same_v<std::decay_t<T>, ElementNode>) {
      if(o->type() == NodeType::ELEMENT) ret = reinterpret_cast<T*>(o);
   }
   if(ret == nullptr) FATAL(format("cast ast mismatch"));
   return ret;
}

template<std::derived_from<AstNode> T>
const T* cast_ast_node(const AstNode* o) noexcept
{
   return cast_ast_node<T>(const_cast<AstNode*>(o));
}

// ------------------------------------------------- recursively visit all nodes

template<std::derived_from<AstNode> T, // T must be a subclass of AstNode
         std::invocable<AstNode*> F>   // f(T*) must work
inline bool preorder_traversal(T* node, F&& f) noexcept
{
   assert(node != nullptr);
   if(!f(node)) return false; // Visit the node
   for(auto&& child : node->children())
      if(!preorder_traversal(child, f)) return false; // Visit all children
   return true;
}

} // namespace giraffe
