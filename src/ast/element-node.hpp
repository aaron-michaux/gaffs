
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
   virtual ~ElementNode() = default;

   bool is_string() const noexcept { return token_.id() == TSTRING; }
   bool is_identifier() const noexcept { return token_.id() == TIDENTIFIER; }
   bool is_element_list() const noexcept { return token_.id() == TNONE; }

   bool has_suffix() const noexcept { return suffix_.id() != TNONE; }
   bool is_star() const noexcept { return suffix_.id() == TSTAR; }
   bool is_plus() const noexcept { return suffix_.id() == TPLUS; }
   bool is_question() const noexcept { return suffix_.id() == TQUESTION; }

   const ElementListNode* element_list() const noexcept
   {
      assert(size() == 0 || size() == 1);
      return (size() == 0)
                 ? nullptr
                 : reinterpret_cast<const ElementListNode*>(children()[0]);
   }

   std::ostream& stream(std::ostream& ss) const noexcept override
   {
      if(is_element_list()) {
         ss << "( " << element_list() << " )" << suffix_.text();
      } else {
         ss << token_.text() << suffix_.text();
      }
      return ss;
   }
};

} // namespace giraffe
