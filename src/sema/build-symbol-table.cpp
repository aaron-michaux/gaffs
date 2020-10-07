
#include "ast/ast.hpp"
#include "build-symbol-table.hpp"
#include "driver/compiler-context.hpp"
#include "symbol-table.hpp"
#include "utils/case-check.hpp"

namespace giraffe
{
namespace build_symbol_table_pass
{
   static void process_element(CompilerContext&, Scope*, ElementNode*);
   static void process_element_list(CompilerContext&, Scope*, ElementListNode*);
   static void process_rule(CompilerContext&, Scope*, RuleNode*);
   static void process_grammar(CompilerContext&, Scope*, GrammarNode*);
   static void find_rule_names(CompilerContext&, Scope*, GrammarNode*);

   static void
   process_element(CompilerContext& context, Scope* scope, ElementNode* node)
   {
      if(node->is_string()) {
         string_view label = text(context, node->token());
         if(!scope->has(label)) scope->insert(label, make_string_symbol(label));
      } else if(node->is_identifier()) {
         string_view label = text(context, node->token());
         assert(!label.empty());
         if(!scope->has(label)) {
            if(!is_uppercase_string(label)) {
               push_warn(context,
                         node->token().location(),
                         "tokens should be uppercase!");
            }
            scope->insert(label, make_token_symbol(label));
         }
      } else if(node->is_element_list()) {
         auto elem_list = node->element_list();
         if(elem_list != nullptr)
            process_element_list(context, scope, elem_list);
      }
   }

   static void process_element_list(CompilerContext& context,
                                    Scope* scope,
                                    ElementListNode* node)
   {
      for(size_t i = 0; i < node->size(); ++i)
         process_element(context, scope, node->element(i));
   }

   static void
   process_rule(CompilerContext& context, Scope* scope, RuleNode* node)
   {
      for(size_t i = 0; i < node->size(); ++i)
         process_element_list(context, scope, node->element_list(i));
   }

   static void
   process_grammar(CompilerContext& context, Scope* scope, GrammarNode* node)
   {
      for(size_t i = 0; i < node->size(); ++i)
         process_rule(context, scope, node->rule(i));
   }

   static void
   find_rule_names(CompilerContext& context, Scope* scope, GrammarNode* node)
   {
      for(size_t i = 0; i < node->size(); ++i) {
         auto rule = node->rule(i);

         const auto& id_tok = rule->identifier();
         string_view label  = text(context, id_tok);
         if(label.empty()) {
            push_error(
                context, id_tok.location(), "rule identifier cannot be empty!");
         } else if(scope->has(label)) {
            push_error(
                context, id_tok.location(), "duplicate rule identifier!");
         } else {
            if(!is_justified_string(label))
               push_warn(context,
                         id_tok.location(),
                         "rule identifiers should be in Justified case");
            scope->insert(label, make_rule_symbol(label, rule));
         }
      }
   }

} // namespace build_symbol_table_pass

unique_ptr<Scope> build_symbol_table(CompilerContext& context,
                                     GrammarNode* grammar) noexcept
{
   assert(grammar != nullptr);

   auto scope = make_unique<Scope>();

   // We want to intern all the rule names before considering other
   // symbols... because we want to be able to distinguish rules
   // even when they appear out of order.
   build_symbol_table_pass::find_rule_names(context, scope.get(), grammar);

   // Find all the other symbols
   build_symbol_table_pass::process_grammar(context, scope.get(), grammar);
   return scope;
}

} // namespace giraffe
