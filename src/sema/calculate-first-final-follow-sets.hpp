

#pragma once

namespace giraffe
{
class CompilerContext;
class GrammarNode;

void calculate_first_final_follow_sets(CompilerContext& context,
                                       GrammarNode* grammar) noexcept;

} // namespace giraffe
