
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// ----------------------------------------------------------------- ElementNode
//
class ElementNode final : public AstNode
{
 private:
   Token token_  = {}; // IDENTIFIER | STRING
   Token suffix_ = {}; // '*' | '+' | '?'

 public:
   ElementNode()
       : AstNode(NodeType::ELEMENT)
   {}
   virtual ~ElementNode() = default;

   bool is_string() const noexcept { return token_.id() == TSTRING; }
   bool is_identifier() const noexcept { return token_.id() == TIDENTIFIER; }
   bool is_element_list() const noexcept { return token_.id() == TLPAREN; }

   bool has_suffix() const noexcept { return suffix_.id() != TNONE; }
   bool is_star() const noexcept { return suffix_.id() == TSTAR; }
   bool is_plus() const noexcept { return suffix_.id() == TPLUS; }
   bool is_question() const noexcept { return suffix_.id() == TQUESTION; }

   /// An `optional` element may, or may not, appear 0 or more times.
   /// That is, it's a `*` or `?` element
   bool is_optional() const noexcept { return is_star() || is_question(); }

   const auto& token() const noexcept { return token_; }

   void set_token(const Token& token) noexcept { token_ = token; }
   void set_suffix(const Token& token) noexcept { suffix_ = token; }

   ElementListNode* element_list() noexcept
   {
      assert(size() == 1);
      return cast_child_<ElementListNode>(0);
   }

   const ElementListNode* element_list() const noexcept
   {
      assert(size() == 1);
      return cast_child_<const ElementListNode>(0);
   }

   std::ostream& stream(std::ostream& ss,
                        const TokenTextFunctor& text) const noexcept override
   {
      if(is_element_list()) {
         ss << "( " << PP{element_list(), text} << " )" << text(suffix_);
      } else {
         ss << text(token_) << text(suffix_);
      }
      return ss;
   }
};

} // namespace giraffe
