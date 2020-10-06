

#pragma once

namespace giraffe
{
class CompilerContext;
class Scope;
class GrammarNode;

void calculate_first_final_follow_sets(CompilerContext& context,
                                       Scope* global_symbols,
                                       GrammarNode* grammar) noexcept;

} // namespace giraffe
