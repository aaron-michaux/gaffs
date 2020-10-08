
#include "ast/ast.hpp"
#include "calculate-first-final-follow-sets.hpp"
#include "driver/compiler-context.hpp"
#include "sema-helpers.hpp"
#include "symbol-table.hpp"

namespace giraffe::find_ll_rules
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
            find_elem_set(
                begin(*elem_list), end(*elem_list), [&](ElementNode* elem) {
                   assert(lookup_.count(elem) == 1);
                   rule_parse_nodes.first_set.push_back(lookup_[elem]);
                });
         }
      }
   }

   { // (2) Remove rules from their own expansion set
      for(auto& rule_parse_node : rules_) {
         const RuleNode* rule = rule_parse_node.rule;
         assert(rule != nullptr);
         auto rule_name = text(*context_, rule->identifier());
         auto ii        = std::partition(
             begin(rule_parse_node.first_set),
             end(rule_parse_node.first_set),
             [&](auto ptr) { return get_name(ptr) == rule_name; });
         rule_parse_node.first_set.erase(ii, end(rule_parse_node.first_set));
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
                  return get_name(ptr) == needle;
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
         const vector<ParseNode*>& expansion = rule_parse_node.first_set;
         for(auto& other_rule_parse_nodes : rules_)
            search_replace(
                other_rule_parse_nodes.first_set, rule_name, expansion);
      };

      for(auto& rule_parse_node : rules_)
         expand_rule_parse_node(rule_parse_node);
   }

   { // (4) Remove Duplicates
      for(auto& rule_parse_nodes : rules_) {
         remove_duplicates(rule_parse_nodes.first_set);
         rule_parse_nodes.first_set.shrink_to_fit();
      }
   }
}

// ------------------------------------------------------------ init-1-elem-list

void ParseGraph::init_1_elem_list_(ElementListNode* elem_list) noexcept
{
   // `lhs` can transition to `rhs`
   auto process_elem_elem = [&](ElementNode* lhs, ElementNode* rhs) {
      assert(!lhs->is_element_list());
      assert(!rhs->is_element_list());
      assert(lookup_.count(lhs) == 1);
      assert(lookup_.count(rhs) == 1);
      lookup_[lhs]->children.push_back(lookup_[rhs]);
   };

   // `lhs` can transition to a token in the range [ii, ee)
   auto process_lhs_to_list = [&](ElementNode* lhs, auto ii, auto ee) {
      assert(!lhs->is_element_list());
      find_elem_set(
          ii, ee, [&](ElementNode* rhs) { process_elem_elem(lhs, rhs); });
   };

   const auto ee = end(*elem_list);
   for(auto ii = begin(*elem_list); ii != ee; ++ii) {
      ElementNode* elem = cast_ast_node<ElementNode>(*ii);
      if(!elem->is_element_list()) {
         // Find the successor nodes, from elem -> rhs
         process_lhs_to_list(elem, std::next(ii), ee);
      } else {
         auto child_elem_list = elem->element_list();
         init_1_elem_list_(child_elem_list); // Process the entire list
         // Find all possible right-hand tokens of `child_elem_list`,
         // which become lhs tokens
         find_elem_set(child_elem_list->rbegin(),
                       child_elem_list->rend(),
                       [&](ElementNode* lhs) {
                          process_lhs_to_list(lhs, std::next(ii), ee);
                       });
      }
   }

   // Remove duplicates "so far"...
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

   // Find all token->token transitions, without expanding rules...
   for(auto& rule_node : *grammar)
      for(auto& elem_list_node : *rule_node)
         init_1_elem_list_(cast_ast_node<ElementListNode>(elem_list_node));
}

static void find_parse_tree(CompilerContext& context,
                            GrammarNode* grammar) noexcept
{
   ParseGraph graph;
   graph.init(context, grammar);
}

namespace detail
{
   template<typename InputItr, std::invocable<size_t, ElementNode*> F>
   static void find_ll_k(InputItr bb,
                         InputItr ee,
                         size_t position, // were we're currently looking
                         size_t k,        // how far to look
                         F&& callback) noexcept
   {
      for(auto ii = bb; ii != ee && position < k; ++ii) {
         ElementNode* elem = cast_ast_node<ElementNode>(*ii);
         if(elem->is_element_list()) {
            // TODO
         } else {
            callback(k, elem);
            find_ll_k(std::next(ii), ee, position + 1, callback);
         }
         if(!elem->is_optional()) break; // we're done
      }
   }
} // namespace detail

template<typename InputItr>
static vector<vector<ElementNode*>>
find_ll_k(InputItr bb, InputItr ee, size_t k) noexcept
{
   vector<vector<ElementNode*>> lists;

   // for(auto ii = bb; ii != ee; ++ii) {
   //    ElementNode* elem = cast_ast_node<ElementNode>(*ii);
   //    if(elem->is_question()) {
   //       // We accept, or skip
   //    }
   // }

   // lists.resize
   // detail::find_ll_k(bb, ee, 0, k, callback);
   return lists;
}

static std::ostream& stream_node_address(std::ostream& ss,
                                         CompilerContext& context,
                                         const AstNode* node)
{
   switch(node->type()) {
   case NodeType::NONE: assert(false); return ss;
   case NodeType::GRAMMAR: return ss;
   case NodeType::RULE:
      ss << text(context, cast_ast_node<RuleNode>(node)->identifier());
      return ss;
   case NodeType::ELEMENTLIST:
      stream_node_address(ss, context, node->parent());
      ss << ':' << node->index_in_parent();
      return ss;
   case NodeType::ELEMENT:
      stream_node_address(ss, context, node->parent());
      ss << ':' << node->index_in_parent();
      return ss;
   }
   return ss;
}

static string element_label(CompilerContext& context, const ElementNode* elem)
{
   std::stringstream ss{""};
   stream_node_address(ss, context, elem);
   ss << ' ' << text(context, elem->token());
   return ss.str();
}

template<std::invocable<AstNode*> F>
inline void traversal(CompilerContext& context,
                      const Scope* scope,
                      AstNode* node,
                      F&& f) noexcept
{
   auto visit_elem = [&](ElementNode* elem) {
      if(elem->is_element_list()) {
         for(AstNode* child : node->children())
            traversal(context, scope, child, f);
      } else if(elem->is_string()) {
         f(elem);
      } else if(elem->is_identifier()) {
         auto symbol = scope->lookup(text(context, elem->token()));
         assert(symbol != nullptr);
         if(symbol->type == Type::RULE) {
            auto rule = cast_ast_node<RuleNode>(symbol->node);
            // MAY have a decision point...
         } else if(symbol->type == Type::TOKEN) {
            f(elem);
         } else {
            assert(false);
         }
      } else {
         assert(false);
      }

      if(elem->has_suffix()) {
         // MAY have a decision point
      }
   };

   assert(node != nullptr);
   switch(node->type()) {
   case NodeType::NONE: assert(false); return;
   case NodeType::GRAMMAR:
      // Traverse the rules
      for(AstNode* child : node->children())
         traversal(context, scope, child, f);
      return;
   case NodeType::RULE:
      // Start traversing the rule's element list... DECISION POINT
      for(AstNode* child : node->children())
         traversal(context, scope, child, f);
      return;
   case NodeType::ELEMENTLIST:
      // Element lists are easy
      for(AstNode* child : node->children())
         traversal(context, scope, child, f);
      return;
   case NodeType::ELEMENT: visit_elem(cast_ast_node<ElementNode>(node)); return;
   }
}

// static void find_parse_tree(CompilerContext& context, GrammarNode* grammar)
// {
//    traversal(context, grammar->scope(), grammar, [&](AstNode* node) {
//       if(node->type() == NodeType::ELEMENT) {
//          auto elem = cast_ast_node<ElementNode>(node);
//          if(!elem->is_element_list())
//             cout << element_label(context, elem) << endl;
//       }
//       return true;
//    });
// }

// static void find_decisions(CompilerContext& context, RuleNode* rule) noexcept
// {
//    // Where are the decision points?
//    // [1] Every alternative in an element list
//    // [2] In an element-list, every '?', '*', '+'... keeping in mind its
//    context
//    // And keep in mind that we can stumble upon ambiguities

//    vector<string_view> expanded_rules;
//    // parse_visit(rule,

//    // auto elem_text = [&context](const ElementNode* elem) -> string_view {
//    //    return text(context, elem->token());
//    // };

//    // cout << format("Rule: {}\n", text(context, rule->identifier()));
//    // size_t counter = 0;
//    // for(AstNode* elem_list_node : *rule) {
//    //    auto lists = find_ll_k(begin(elem_list_node), end(elem_list_node),
//    2);

//    //    { // Some printing
//    //       cout << format("   [{}]\n", counter++);
//    //       for(const auto& list : lists) {
//    //          cout << format("      {}\n",
//    //                         implode(begin(list), end(list), ", ",
//    elem_text));
//    //       }
//    //    }
//    // }
// }

} // namespace giraffe::find_ll_rules

namespace giraffe
{
// ---------------------------------------------------------- calculate ll rules

void calculate_ll_rules(CompilerContext& context, GrammarNode* grammar) noexcept
{
   find_ll_rules::find_parse_tree(context, grammar);
   // for(AstNode* rule : *grammar)
   //    find_ll_rules::find_decisions(context, cast_ast_node<RuleNode>(rule));
}

} // namespace giraffe
