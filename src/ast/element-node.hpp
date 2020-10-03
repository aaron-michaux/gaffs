
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
   bool is_element_list() const noexcept { return token_.id() == TLPAREN; }

   bool has_suffix() const noexcept { return suffix_.id() != TNONE; }
   bool is_star() const noexcept { return suffix_.id() == TSTAR; }
   bool is_plus() const noexcept { return suffix_.id() == TPLUS; }
   bool is_question() const noexcept { return suffix_.id() == TQUESTION; }

   void set_token(const Token& token) noexcept { token_ = token; }
   void set_suffix(const Token& token) noexcept { suffix_ = token; }

   const ElementListNode* element_list() const noexcept
   {
      assert(size() == 0 || size() == 1);
      return (size() == 0)
                 ? nullptr // parse errors can cause this
                 : reinterpret_cast<const ElementListNode*>(children()[0]);
   }

   std::ostream& stream(std::ostream& ss,
                        string_view buffer) const noexcept override
   {
      if(is_element_list()) {
         ss << "( " << PP{element_list(), buffer} << " )"
            << suffix_.text(buffer);
      } else {
         ss << token_.text(buffer) << suffix_.text(buffer);
      }
      return ss;
   }
};

} // namespace giraffe
