
#include "compiler-context.hpp"

#define This CompilerContext

namespace giraffe
{
// -----------------------------------------------------------------------
// stream

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

// ------------------------------------------------------------------------ text

string_view text(const CompilerContext& context, const Token& token) noexcept
{
   assert(token.key() < context.texts.size());
   const auto& ss = context.texts.at(token.key());
   assert(token.offset() <= ss.size());
   assert(token.offset() + token.length() <= ss.size());
   return string_view(&ss[token.offset()], token.length());
}

// ------------------------------------------------------- make-compiler-context

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

// ------------------------------------------------------------------ push-error

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

// ---------------------------------------------------------------- push-warning

void push_warn(CompilerContext& context,
               SourceLocation location,
               string&& message) noexcept
{
   if(context.options.w_error) {
      push_error(context, location, std::move(message));
   } else {
      context.diagnostics.emplace_back(
          Diagnostic::WARN, location, std::move(message));
      context.n_warnings += 1;
   }
}

void push_warn(CompilerContext& context, string&& message) noexcept
{
   push_warn(context, context.tokens.current().location(), std::move(message));
}

} // namespace giraffe

#undef This
