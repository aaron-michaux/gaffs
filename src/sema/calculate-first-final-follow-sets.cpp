
#include "ast/ast.hpp"
#include "calculate-first-final-follow-sets.hpp"
#include "driver/compiler-context.hpp"
#include "sema-helpers.hpp"

namespace giraffe::sema::find_first_final_follow_sets_pass
{
template<typename U, typename V> static void append(U& dst, const V& src)
{
   dst.insert(end(dst), cbegin(src), cend(src));
}

struct FFFSets
{
   vector<string_view> first_set;
   vector<string_view> final_set;
   vector<string_view> follow_set;
};

struct PassContext
{
   CompilerContext& context;
   GrammarNode* grammar;

   vector<FFFSets> sets;

   PassContext(CompilerContext& ctx, GrammarNode* g)
       : context(ctx)
       , grammar(g)
   {
      sets.resize(g->size());
   }
};

/// Find the first and final sets of each rule, WITHOUT expanding any rules
static void calculate(CompilerContext& context, GrammarNode* grammar)
{
   auto pctx  = PassContext{context, grammar};
   auto scope = grammar->scope();

   auto symbol_of = [scope, &context](const ElementNode* elem) {
      const auto name = text(context, elem->token());
      auto symbol     = scope->lookup(name);
      assert(symbol != nullptr);
      return symbol;
   };

   auto elem_to_rule = [&](const ElementNode* elem) -> RuleNode* {
      if(!elem->is_identifier()) return nullptr;
      auto symbol = symbol_of(elem);
      if(symbol->type != Type::RULE) return nullptr;
      return cast_ast_node<RuleNode>(symbol->node);
   };

   auto sets_of_rule = [&](AstNode * node) -> auto&
   {
      auto rule = cast_ast_node<RuleNode>(node);
      return pctx.sets.at(rule->index_in_parent());
   };

   auto expand_all_rules_for_set = [&](auto&& get_set) {
      auto expand_rules = [&](RuleNode* rule) {
         auto& rule_set       = get_set(rule);
         const auto rule_name = text(context, rule->identifier());

         // Remove the rule name from its own set
         rule_set.erase(std::remove(begin(rule_set), end(rule_set), rule_name),
                        end(rule_set));

         // Iterate over all sets, replacing the rule with its `set`
         for(AstNode* r_node : *grammar) {
            auto& set = get_set(r_node);
            auto ii   = std::remove(begin(set), end(set), rule_name);
            if(ii != end(set)) {
               set.erase(ii, end(set));
               set.insert(end(set), begin(rule_set), end(rule_set));
            }
         }
      };

      // Expand sets for each rule
      for(AstNode* r_node : *grammar)
         expand_rules(cast_ast_node<RuleNode>(r_node));
   };

   // ---------------------------- Calculating first/final sets
   // (1) Add the first and final tokens to the first/final sets
   // (2) Expand `rule` tokens to be their First/Final sets
   // (3) Remove duplicates

   { // (1) Add the first and final tokens to the first/final sets
      for(AstNode* r_node : *grammar) {
         auto rule  = cast_ast_node<RuleNode>(r_node);
         auto& sets = sets_of_rule(rule);

         // Calculate `first` and `final` sets
         for(AstNode* el_node : *rule) {
            auto elem_list = cast_ast_node<ElementListNode>(el_node);

            find_elem_set(
                begin(*elem_list), end(*elem_list), [&](ElementNode* elem) {
                   sets.first_set.push_back(text(context, elem->token()));
                });

            find_elem_set(
                rbegin(*elem_list), rend(*elem_list), [&](ElementNode* elem) {
                   sets.final_set.push_back(text(context, elem->token()));
                });
         }
      }
   }

   { // (2) Replace expand `rule` tokens to be their First sets
      expand_all_rules_for_set([&](AstNode * node) -> auto& {
         return sets_of_rule(node).first_set;
      });
      expand_all_rules_for_set([&](AstNode * node) -> auto& {
         return sets_of_rule(node).final_set;
      });
   }

   { // (3) Remove duplicates
      for(AstNode* r_node : *grammar) {
         auto& sets = sets_of_rule(r_node);
         remove_duplicates(sets.first_set);
         remove_duplicates(sets.final_set);
         sets.first_set.shrink_to_fit();
         sets.final_set.shrink_to_fit();
      }
   }

   // ---------------------- Calculating follow-sets
   // (A) Rule: ... a Lhs ;   => Add Follow(Rule) to Follow(Lhs)
   // (B) Rule: ... A* ... ;  => A can follow itself, Add First(A) to Follow(A)
   // (C) Rule: ... R a ... ; => Add First(a) to Follow(R)
   // (D) Expand `rule` tokens to be their follow sets
   // (E) Remove duplicates

   { // (A) If an element-list ends with a production rule...
      //     Then we add that rule's follow set to this rule.
      for(AstNode* r_node : *grammar) { // Calculate `follow` sets
         auto rule = cast_ast_node<RuleNode>(r_node);
         for(AstNode* el_node : *rule) {
            auto elem_list = cast_ast_node<ElementListNode>(el_node);
            find_elem_set(
                rbegin(*elem_list), rend(*elem_list), [&](ElementNode* elem) {
                   if(auto symbol = symbol_of(elem);
                      symbol->type == Type::RULE) {
                      sets_of_rule(cast_ast_node<RuleNode>(symbol->node))
                          .follow_set.push_back(
                              text(context, rule->identifier()));
                   }
                });
         }
      }
   }

   { // (B) If any rule is '+' or '*', then they automatically can follow
     // themselves
      preorder_traversal(grammar, [&](AstNode* node) -> bool {
         if(node->type() == NodeType::ELEMENT) {
            auto elem = cast_ast_node<ElementNode>(node);
            if(elem->is_star() || elem->is_plus()) {
               auto elem_rule = elem_to_rule(elem);
               if(elem_rule != nullptr) {
                  auto& sets = sets_of_rule(elem_rule);
                  append(sets.follow_set, sets.first_set);
               }
            }
         }
         return true;
      });
   }

   { // (C) If we have a sequence `R a`, for rule `R`, then
      //     add the first set of `a` to the follow set of `R`
      auto add_to_follow_set = [&](RuleNode* rule, ElementNode* rhs) {
         auto& rule_sets = sets_of_rule(rule); // Sets that we add to
         assert(!rhs->is_element_list());
         auto name   = text(context, rhs->token());
         auto symbol = scope->lookup(name);
         assert(symbol);
         switch(symbol->type) {
         case Type::NONE: assert(false); break;
         case Type::RULE:                // Append the rhs' first set
            append(rule_sets.follow_set, // to `rule` follow set
                   sets_of_rule(elem_to_rule(rhs)).first_set);
            break;
         case Type::TOKEN: rule_sets.follow_set.push_back(name); break;
         case Type::STRING: rule_sets.follow_set.push_back(name); break;
         }
      };

      /// Examine an adjacent pair for (C) above. `ii`, and `jj` are iterators
      auto process_elem_pair = [&](auto ii, auto jj) {
         auto elem_list = cast_ast_node<ElementListNode>((*ii)->parent());
         auto rr        = std::make_reverse_iterator(std::next(ii));
         auto rr_end    = std::make_reverse_iterator(ii);

         // If *ii is an element-list, we need to search from the right
         find_elem_set(rr, rr_end, [&](ElementNode* lhs) {
            auto lhs_rule = elem_to_rule(lhs);
            if(lhs_rule != nullptr) { // We have a rule, find a `rhs` match
               find_elem_set(jj, end(*elem_list), [&](ElementNode* rhs) {
                  add_to_follow_set(lhs_rule, rhs);
               });
            }
         });
      };

      /// Action, examine adjacent elements in __EVERY__ elem list
      preorder_traversal(grammar, [&](AstNode* node) -> bool {
         if(node->type() == NodeType::ELEMENTLIST)
            for_each_adjacent_pair(begin(*node), end(*node), process_elem_pair);
         return true;
      });
   }

   {
      // (D) Expand `rule` tokens to be their follow sets
      expand_all_rules_for_set([&](AstNode * node) -> auto& {
         return sets_of_rule(node).follow_set;
      });
   }

   { // (E) Remove duplicates
      for(AstNode* r_node : *grammar) {
         auto& sets = sets_of_rule(r_node);
         remove_duplicates(sets.follow_set);
         sets.follow_set.shrink_to_fit();
      }
   }

   // Finally: update the first/final/follow sets in the AST.
   for(AstNode* r_node : *grammar) {
      auto rule  = cast_ast_node<RuleNode>(r_node);
      auto& sets = pctx.sets.at(rule->index_in_parent());

      rule->set_first_set(sets.first_set);
      rule->set_final_set(sets.final_set);
      rule->set_follow_set(sets.follow_set);

      if(false) {
         auto print_set
             = [&](auto& set) { return implode(cbegin(set), cend(set), ", "); };
         TRACE(format("Rule: {}", text(context, rule->identifier())));
         cout << format("First Set:  {}\n", print_set(sets.first_set));
         cout << format("Final Set:  {}\n", print_set(sets.final_set));
         cout << format("Follow Set: {}\n", print_set(sets.follow_set));
         cout << endl;
      }
   }
}

} // namespace giraffe::sema::find_first_final_follow_sets_pass

namespace giraffe
{
// ------------------------------------------- calculate first final follow sets

void calculate_first_final_follow_sets(CompilerContext& context,
                                       GrammarNode* grammar) noexcept
{
   if(grammar->scope() == nullptr) {
      FATAL(format("must build the symbol table before calculating "
                   "first/final/follow sets"));
   }

   // Get the first/final sets, without expanding rules...
   sema::find_first_final_follow_sets_pass::calculate(context, grammar);
}
} // namespace giraffe
