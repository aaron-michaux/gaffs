
#include "ast/ast.hpp"
#include "calculate-first-final-follow-sets.hpp"
#include "driver/compiler-context.hpp"
#include "symbol-table.hpp"

namespace giraffe::find_ll_rules
{
static void calculate(CompilerContext& context, GrammarNode* grammar)
{
   // Where are the decision points?
   // [1] Every alternative in an element list
   // [2] In an element-list, every '?', '*', '+'... keeping in mind its context
   // And keep in mind that we're searching for ambiguities

   // for(AstNode* rule : *grammar)
   //    find_ll_rules(context, cast_ast_node<RuleNode>(rule));
}

} // namespace giraffe::find_ll_rules

namespace giraffe
{
// ---------------------------------------------------------- calculate ll rules

void calculate_ll_rules(CompilerContext& context, GrammarNode* grammar) noexcept
{
   find_ll_rules::calculate(context, grammar);
}

} // namespace giraffe
