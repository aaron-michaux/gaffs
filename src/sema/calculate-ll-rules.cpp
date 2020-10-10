
#include "ast/ast.hpp"
#include "calculate-first-final-follow-sets.hpp"
#include "driver/compiler-context.hpp"
#include "sema-helpers.hpp"

namespace giraffe::sema::find_ll_rules
{
static bool node_of_interest(const AstNode* node) noexcept
{
   return node->type() == NodeType::ELEMENT
          && !cast_ast_node<ElementNode>(node)->is_element_list();
}

static size_t count_nodes_of_interest(GrammarNode* grammar) noexcept
{
   size_t counter = 0;
   preorder_traversal(grammar, [&](AstNode* node) {
      if(node_of_interest(node)) ++counter;
      return true;
   });
   return counter;
}

class ParseGraph
{
 public:
   struct ParseNode final
   {
      const ElementNode* elem     = nullptr; // a rule or element
      vector<ParseNode*> children = {};      // non-owning
   };

   struct RuleParseNodes final
   {
      const RuleNode* rule;
      vector<ParseNode*> first_set = {};
      vector<ParseNode*> final_set = {};
   };

 private:
   CompilerContext* context_                                  = nullptr;
   Scope* scope_                                              = nullptr;
   vector<ParseNode> nodes_                                   = {};
   vector<RuleParseNodes> rules_                              = {};
   std::unordered_map<const ElementNode*, ParseNode*> lookup_ = {};

   RuleNode* elem_to_rule(const ElementNode* elem) const noexcept
   {
      if(!elem->is_identifier()) return nullptr;
      auto symbol = scope_->lookup(text(*context_, elem->token()));
      assert(symbol != nullptr);
      return (symbol->type != Type::RULE)
                 ? nullptr
                 : cast_ast_node<RuleNode>(symbol->node);
   }

   void init_0_rules_(GrammarNode*) noexcept;
   void init_1_elem_list_(ElementListNode*) noexcept;

 public:
   void init(CompilerContext& context, GrammarNode* grammar) noexcept;
   const auto& rules() const noexcept { return rules_; }
};

// ----------------------------------------------------------------- init-0-rule

void ParseGraph::init_0_rules_(GrammarNode* grammar) noexcept
{
   assert(context_ != nullptr);
   rules_.clear();
   rules_.resize(grammar->size());

   // Helper function
   // Get the name for the rule pointed to by `node`, or "" if none.
   auto get_name = [&](const ParseNode* node) -> string_view {
      auto rule = elem_to_rule(node->elem);
      return (rule == nullptr) ? "" : text(*context_, rule->identifier());
   };

   // --------------------------------------------------- //
   // (1) Map rule to ParseNodes, without expanding rules //
   // (2) Remove rules from their own expansion set       //
   // (3) Expand rules                                    //
   // (4) Remove duplicates                               //
   // --------------------------------------------------- //
   { // (1) Map rules to their ParseNodes, without expanding rules...
      for(AstNode* rule_node : *grammar) {
         const size_t index = rule_node->index_in_parent();
         assert(index < rules_.size());
         auto& rule_parse_nodes = rules_[index];
         rule_parse_nodes.rule  = cast_ast_node<RuleNode>(rule_node);
         for(AstNode* el_node : *rule_node) {
            auto elem_list = cast_ast_node<ElementListNode>(el_node);

            // The first sets
            find_elem_set(
                begin(*elem_list), end(*elem_list), [&](ElementNode* elem) {
                   assert(lookup_.count(elem) == 1);
                   rule_parse_nodes.first_set.push_back(lookup_[elem]);
                });

            // The final sets
            find_elem_set(
                rbegin(*elem_list), rend(*elem_list), [&](ElementNode* elem) {
                   assert(lookup_.count(elem) == 1);
                   rule_parse_nodes.final_set.push_back(lookup_[elem]);
                });
         }
      }
   }

   { // (2) Remove rules from their own expansion set
      auto remove_it = [&](string_view rule_name, auto& set) {
         set.erase(std::partition(
                       begin(set),
                       end(set),
                       [&](auto ptr) { return get_name(ptr) != rule_name; }),
                   end(set));
      };

      for(auto& rule_parse_node : rules_) {
         const RuleNode* rule = rule_parse_node.rule;
         assert(rule != nullptr);
         auto rule_name = text(*context_, rule->identifier());
         remove_it(rule_name, rule_parse_node.first_set);
         remove_it(rule_name, rule_parse_node.final_set);
      }
   }

   { // (3) Expand rules
      // Search `haystack` for rules named `needle`. If found, insert
      // the range of elements [replace_bb..replace_ee)
      auto search_replace = [&](vector<ParseNode*>& haystack,
                                string_view needle,
                                const vector<ParseNode*>& expansion) {
         auto ii
             = std::partition(begin(haystack), end(haystack), [&](auto ptr) {
                  return get_name(ptr) != needle;
               });
         if(ii != end(haystack)) {
            // We found the rule at least once, so remove the rule,
            // and add in the "replace" set
            haystack.erase(ii, end(haystack));
            haystack.insert(end(haystack), begin(expansion), end(expansion));
         }
      };

      // Expand search replace all rules for the passed rule
      auto expand_rule_parse_node = [&](const RuleParseNodes& rule_parse_node) {
         // What rule are we dealing with?
         const RuleNode* rule = rule_parse_node.rule;
         auto rule_name       = text(*context_, rule->identifier());

         for(auto& other_rule_parse_nodes : rules_) {
            // First sets
            search_replace(other_rule_parse_nodes.first_set,
                           rule_name,
                           rule_parse_node.first_set);

            // Final sets
            search_replace(other_rule_parse_nodes.final_set,
                           rule_name,
                           rule_parse_node.final_set);
         }
      };

      for(auto& rule_parse_node : rules_)
         expand_rule_parse_node(rule_parse_node);
   }

   { // (4) Remove Duplicates
      for(auto& rule_parse_node : rules_) {
         remove_duplicates(rule_parse_node.first_set);
         remove_duplicates(rule_parse_node.final_set);
         rule_parse_node.first_set.shrink_to_fit();
         rule_parse_node.final_set.shrink_to_fit();
      }
   }
}

// ------------------------------------------------------------ init-1-elem-list

void ParseGraph::init_1_elem_list_(ElementListNode* elem_list) noexcept
{
   // `lhs` can transition to `rhs`
   auto process_elem_elem = [&](const ElementNode* lhs,
                                const ElementNode* rhs) {
      assert(!lhs->is_element_list());
      assert(!rhs->is_element_list());
      assert(lookup_.count(lhs) == 1);
      assert(elem_to_rule(lhs) == nullptr);

      RuleNode* rhs_rule = elem_to_rule(rhs);
      if(rhs_rule == nullptr) {
         assert(lookup_.count(rhs) == 1);
         lookup_[lhs]->children.push_back(lookup_[rhs]);
      } else {
         auto& first_set = rules_.at(rhs_rule->index_in_parent()).first_set;
         for(auto first_node : first_set) {
            auto rhs_elem = first_node->elem;
            assert(!rhs_elem->is_element_list());
            assert(elem_to_rule(rhs_elem) == nullptr);
            assert(lookup_.count(rhs_elem) == 1);
            lookup_[lhs]->children.push_back(lookup_[rhs_elem]);
         }
      }
   };

   // `lhs` can transition to a token in the range [ii, ee)
   auto process_atom_lhs_to_list = [&](const auto lhs, auto ii, auto ee) {
      assert(!lhs->is_element_list());
      assert(elem_to_rule(lhs) == nullptr);
      find_elem_set(
          ii, ee, [&](ElementNode* rhs) { process_elem_elem(lhs, rhs); });
   };

   auto process_lhs_to_list = [&](const ElementNode* lhs, auto ii, auto ee) {
      assert(!lhs->is_element_list());
      RuleNode* rule = elem_to_rule(lhs);
      if(rule == nullptr) {
         process_atom_lhs_to_list(lhs, ii, ee);
      } else {
         // Find all possible `final` tokens in the rule.
         // These become `lhs` for further search.
         const auto& final_set = rules_.at(rule->index_in_parent()).final_set;
         for(auto final_node : final_set)
            process_atom_lhs_to_list(final_node->elem, ii, ee);
      }
   };

   const auto ee = end(*elem_list);
   for(auto ii = begin(*elem_list); ii != ee; ++ii) {
      ElementNode* elem = cast_ast_node<ElementNode>(*ii);
      RuleNode* rule    = elem_to_rule(elem); // could be null

      const bool is_list = elem->is_element_list();

      if(!is_list) {
         // Find the successor nodes, from elem -> rhs
         process_lhs_to_list(elem, std::next(ii), ee);

      } else if(is_list) {
         // [1] Recursively process child list
         auto child_elem_list = elem->element_list();
         init_1_elem_list_(child_elem_list);

         // [2] Now find all possible right-hand tokens of `child_elem_list`,
         //     These tokens become the `lhs` tokens for further search...
         find_elem_set(child_elem_list->rbegin(),
                       child_elem_list->rend(),
                       [&](ElementNode* lhs) {
                          process_lhs_to_list(lhs, std::next(ii), ee);
                       });

      } else {
         // opps
         assert(false);
      }
   }

   // Remove duplicates...
   for(auto& node : nodes_) remove_duplicates(node.children);
}

// ------------------------------------------------------------------------ init

void ParseGraph::init(CompilerContext& context, GrammarNode* grammar) noexcept
{
   // Initialize
   context_ = &context;
   scope_   = grammar->scope();

   // Create the ParseNodes
   nodes_.clear();
   nodes_.reserve(count_nodes_of_interest(grammar));
   preorder_traversal(grammar, [&](AstNode* node) {
      if(node_of_interest(node))
         nodes_.push_back({cast_ast_node<ElementNode>(node), {}});
      return true;
   });

   // Initialize the lookup
   lookup_.clear();
   lookup_.reserve(nodes_.size());
   for(auto& node : nodes_) lookup_[node.elem] = &node;
   assert(nodes_.size() == lookup_.size()); // otherwise Elements aren't unique!

   // Map rules to their ParseNodes
   init_0_rules_(grammar);

   // Find all token->token transitions, expanding rules...
   for(auto& rule_node : *grammar)
      for(auto& elem_list_node : *rule_node)
         init_1_elem_list_(cast_ast_node<ElementListNode>(elem_list_node));
}

// -------------------------------------------------------- traversing the graph

template<typename F>
static void traverse_worker(CompilerContext& context,
                            ParseGraph::ParseNode* node,
                            size_t current_depth,
                            size_t max_depth,
                            vector<string_view>& elem_seq,
                            F&& callback)
{
   if(current_depth == max_depth) { // depth limit reached
      callback(elem_seq);

   } else if(node->children.size() == 0) { // a terminal
      elem_seq.resize(current_depth);
      callback(elem_seq);

   } else { // recurse
      elem_seq.push_back(text(context, node->elem->token()));
      for(auto child : node->children) {
         traverse_worker(
             context, child, current_depth + 1, max_depth, elem_seq, callback);
      }
      elem_seq.pop_back();
   }
}

template<typename F>
static void traverse(CompilerContext& context,
                     ParseGraph::ParseNode* node,
                     size_t depth,
                     F&& callback)
{
   vector<string_view> elem_seq;
   elem_seq.reserve(depth);
   traverse_worker(context, node, 0, depth, elem_seq, callback);
   assert(elem_seq.empty());
}

// ------------------------------------------------------------- find parse tree

static void find_parse_tree(CompilerContext& context,
                            GrammarNode* grammar) noexcept
{
   ParseGraph graph;
   graph.init(context, grammar);

   if(false) {
      const int depth   = 1;
      string_view delim = "\n              ";
      for(auto& rule : graph.rules()) {
         vector<vector<string_view>> first_sets;
         for(auto pnode : rule.first_set) {
            traverse(context, pnode, depth, [&](const auto& seq) {
               first_sets.push_back(seq);
            });
         }

         remove_duplicates(first_sets);

         cout << "Rule: " << text(context, rule.rule->identifier()) << '\n';
         cout << "   First Set: ";

         bool is_first = true;
         for(auto set : first_sets) {
            if(is_first) {
               is_first = false;
            } else {
               cout << delim;
            }
            cout << "{ " << implode(cbegin(set), cend(set), ", ") << " }";
         }
         cout << "\n\n";
      }
   }
}

} // namespace giraffe::sema::find_ll_rules

namespace giraffe
{
// ---------------------------------------------------------- calculate ll rules

void calculate_ll_rules(CompilerContext& context, GrammarNode* grammar) noexcept
{
   sema::find_ll_rules::find_parse_tree(context, grammar);
}

} // namespace giraffe
