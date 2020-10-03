
#include "compiler-context.hpp"

#define This CompilerContext

namespace giraffe
{
std::ostream& This::stream(std::ostream& ss) const noexcept
{
   ss << format(R"V0G0N(
CompilerContext
   n-texts:    {}
   n-infos:    {}
   n-warnings: {}
   n-errors:   {}
   n-fatal:    {}
)V0G0N",
                texts.size(),
                n_infos,
                n_warnings,
                n_errors,
                n_fatal);

   for(const auto& diagnostic : diagnostics) diagnostic.stream(ss, *this);

   return ss;
}

CompilerContext make_compiler_context(string_view text, CompilerOptions opts)
{
   CompilerContext context;
   context.options = opts;
   context.names.push_back("<string>");
   context.texts.emplace_back(cbegin(text), cend(text));
   context.tokens.initialize(
       context.texts.size(),
       [&context](size_t index) {
          assert(index < context.texts.size());
          return string_view(context.texts[index]);
       },
       context.options.scanner_options);
   return context;
}

void push_error(CompilerContext& context,
                SourceLocation location,
                string&& message) noexcept
{
   context.diagnostics.emplace_back(
       Diagnostic::ERROR, location, std::move(message));
   context.n_errors += 1;
}

void push_error(CompilerContext& context, string&& message) noexcept
{
   push_error(context, context.tokens.current().location(), std::move(message));
}

} // namespace giraffe

#undef This
