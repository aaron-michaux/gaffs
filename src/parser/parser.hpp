
#include "ast/ast.hpp"

namespace giraffe
{
struct CompilerContext;
struct TokenProducer;

class GrammarNode;

unique_ptr<GrammarNode> parse(CompilerContext& context,
                              TokenProducer& tokens) noexcept;
}; // namespace giraffe
