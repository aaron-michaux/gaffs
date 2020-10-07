
#include "ast/ast.hpp"

namespace giraffe
{
class CompilerContext;

unique_ptr<GrammarNode> parse(CompilerContext& context) noexcept;
} // namespace giraffe
