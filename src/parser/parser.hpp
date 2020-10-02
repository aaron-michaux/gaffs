
#include "ast/ast.hpp"

namespace giraffe
{
struct CompilerContext;
struct Scanner;

class GrammarNode;

unique_ptr<GrammarNode> parse(CompilerContext& context,
                              Scanner& tokens) noexcept;
}; // namespace giraffe
