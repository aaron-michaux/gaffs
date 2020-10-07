
#include "compiler-context.hpp"
#include "parser/parser.hpp"
#include "scanner/scanner.hpp"
#include "sema/sema.hpp"

#define This CompilerContext

namespace giraffe
{
// ---------------------------------------------------------------------- stream

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

unique_ptr<CompilerContext> make_compiler_context(string_view text,
                                                  CompilerOptions opts)
{
   vector<string> names = {"-"s};
   vector<string> texts = {string(cbegin(text), cend(text))};
   return make_compiler_context(std::move(names), std::move(texts), opts);
}

unique_ptr<CompilerContext> make_compiler_context(vector<string>&& names,
                                                  vector<string>&& texts,
                                                  CompilerOptions opts)
{
   assert(names.size() == texts.size());

   auto context           = make_unique<CompilerContext>();
   context->compiler_opts = opts;
   context->names         = std::move(names);
   context->texts         = std::move(texts);
   context->tokens.initialize(
       context->texts.size(),
       [ptr = context.get()](size_t index) {
          assert(index < ptr->texts.size());
          return string_view(ptr->texts[index]);
       },
       context->scanner_opts);
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
   if(context.compiler_opts.w_error) {
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

// --------------------------------------------------------------------- execute

bool execute(CompilerContext& context) noexcept // once set upt
{
   // Parse the grammar
   auto ast = parse(context);

   // Calculculate the global scope
   ast->set_scope(build_symbol_table(context, ast.get()));

   // Finst/final/follow sets
   calculate_first_final_follow_sets(context, ast.get());

   // LL (i.e., parse) rules
   calculate_ll_rules(context, ast.get());

   // Print diagnostics
   std::for_each(
       cbegin(context.diagnostics),
       cend(context.diagnostics),
       [&](const auto& diagnostic) { diagnostic.stream(cout, context); });

   if(context.n_errors > 0) {
      return false; // we failed!
   }

   // Output some nice friendly stuff
   cout << '\n';
   ast->scope()->stream(cout);
   ast->stream(cout, context.tokens.current_buffer());

   for(AstNode* r_node : *ast) {
      auto rule = cast_ast_node<RuleNode>(r_node);
      auto print_set
          = [&](auto& set) { return implode(cbegin(set), cend(set), ", "); };
      cout << format("Rule: {}\n", text(context, rule->identifier()));
      cout << format("   first Set:  {}\n", print_set(rule->first_set()));
      cout << format("   final Set:  {}\n", print_set(rule->final_set()));
      cout << format("   follow Set: {}\n", print_set(rule->follow_set()));
      cout << endl;
   }
   return true; // we're winners
}

} // namespace giraffe

#undef This
