
#pragma once

namespace giraffe
{
class CompilerContext;
class Scope;
class GrammarNode;

unique_ptr<Scope> build_symbol_table(CompilerContext& context,
                                     GrammarNode* root) noexcept;

} // namespace giraffe
