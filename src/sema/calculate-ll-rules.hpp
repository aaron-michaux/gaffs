
#pragma once

namespace giraffe
{
// ---------------------------------------------------------- calculate ll rules
//
/**
 * Build a "parse tree" that shows how (concrete) tokens can transition
 * to other tokens. Each token is related to an `ElementNode` that actually
 * consumes a token. (So not a Rule, and not an element-list.)
 *
 * In theory, this graph can be used to build find first-sets for an LL(k)
 * grammar, or to make a parser generator.
 */
void calculate_ll_rules(CompilerContext& context,
                        GrammarNode* grammar) noexcept;

} // namespace giraffe
