
#include "compiler-context.hpp"

namespace giraffe
{
void push_error(CompilerContext& context,
                SourceLocation location,
                string&& message) noexcept
{
   context.diagnostics.emplace_back(
       Diagnostic::ERROR, location, std::move(message));
   context.n_errors += 1;
}

} // namespace giraffe
