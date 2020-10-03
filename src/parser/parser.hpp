
#include "ast/ast.hpp"

namespace giraffe
{
struct CompilerContext;

unique_ptr<GrammarNode> parse(CompilerContext& context) noexcept;
}; // namespace giraffe
