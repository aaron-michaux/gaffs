
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

   std::ostream& stream(std::ostream& ss,
                        string_view buffer) const noexcept override
   {
      constexpr const char* tab = "      ";
      ss << identifier_.text(buffer) << ":";
      switch(size()) {
      case 0: ss << " ;"; break;
      case 1: ss << ' ' << PP{children()[0], buffer} << " ;"; break;
      default:
         for(size_t i = 0; i < size(); ++i) {
            if(i == 0)
               ss << " ";
            else
               ss << '\n' << tab << "| ";
            ss << PP{children()[i], buffer};
         }
         ss << '\n' << tab << ';';
      }
      ss << "\n";
      return ss;
   }
};
} // namespace giraffe
