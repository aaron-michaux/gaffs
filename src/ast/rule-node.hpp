
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

   vector<string_view> first_set_  = {}; // Tokens the rule can start with
   vector<string_view> final_set_  = {}; // Tokens that can end the rule
   vector<string_view> follow_set_ = {}; // Tokens that can follow the rule

 public:
   RuleNode()
       : AstNode(NodeType::RULE)
   {}
   virtual ~RuleNode() = default;

   const Token& identifier() const noexcept { return identifier_; }

   void set_identifier(const Token& id) noexcept { identifier_ = id; }

   ElementListNode* element_list(size_t index) noexcept
   {
      return cast_child_<ElementListNode>(index);
   }

   const ElementListNode* element_list(size_t index) const noexcept
   {
      return cast_child_<const ElementListNode>(index);
   }

   const auto& first_set() const noexcept { return first_set_; }
   const auto& final_set() const noexcept { return final_set_; }
   const auto& follow_set() const noexcept { return follow_set_; }

   void set_first_set(auto set) noexcept { first_set_ = std::move(set); }
   void set_final_set(auto set) noexcept { final_set_ = std::move(set); }
   void set_follow_set(auto set) noexcept { follow_set_ = std::move(set); }

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
