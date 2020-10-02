
#pragma once

#include "ast-node.hpp"

namespace giraffe
{
// -------------------------------------------------------------------- RuleNode
//
class RuleNode final : public AstNode
{
 private:
   Token identifier_ = {};

 public:
   virtual ~RuleNode() = default;

   const Token& identifier() const noexcept { return identifier_; }

   void set_identifier(const Token& id) noexcept { identifier_ = id; }

   const ElementListNode* element_list(size_t index) const noexcept
   {
      assert(index < size());
      return reinterpret_cast<const ElementListNode*>(children()[index]);
   }

   std::ostream& stream(std::ostream& ss) const noexcept override
   {
      constexpr const char* tab = "      ";
      ss << identifier_.text() << ":";
      switch(size()) {
      case 0: ss << " ;"; break;
      case 1: ss << ' ' << children()[0] << " ;"; break;
      default:
         for(size_t i = 0; i < size(); ++i)
            ss << '\n' << tab << (i == 0 ? "  " : "| ") << children()[i];
         ss << '\n' << tab << ';';
      }
      ss << "\n";
      return ss;
   }
};
} // namespace giraffe
