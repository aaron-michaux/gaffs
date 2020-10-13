
#include "compiler-context.hpp"
#include "parser/parser.hpp"
#include "scanner/scanner.hpp"
#include "sema/sema.hpp"

#define This CompilerContext

namespace giraffe
{
// ------------------------------------------------------- make-compiler-context

unique_ptr<CompilerContext> This::make(string_view text, CompilerOptions opts)
{
   vector<string> names = {"-"s};
   vector<string> texts = {string(cbegin(text), cend(text))};
   return make(std::move(names), std::move(texts), opts);
}

unique_ptr<CompilerContext>
This::make(vector<string>&& names, vector<string>&& texts, CompilerOptions opts)
{
   assert(names.size() == texts.size());

   auto context            = unique_ptr<CompilerContext>(new CompilerContext{});
   context->compiler_opts_ = opts;
   context->names_         = std::move(names);
   context->texts_         = std::move(texts);
   context->scanner_.initialize(
       context->texts().size(),
       [ptr = context.get()](size_t index) {
          assert(index < ptr->texts().size());
          return string_view(ptr->texts()[index]);
       },
       context->scanner_opts_);
   return context;
}

// ------------------------------------------------------------ push diagnostics

void This::push_diagnostic_(Diagnostic::Level level,
                            SourceLocation location,
                            string&& message) noexcept
{
   if(level == Diagnostic::WARN && compiler_opts().w_error)
      level = Diagnostic::ERROR;

   switch(level) {
   case Diagnostic::NONE: assert(false); break;
   case Diagnostic::INFO: ++n_infos_; break;
   case Diagnostic::WARN: ++n_warns_; break;
   case Diagnostic::ERROR: ++n_errors_; break;
   case Diagnostic::FATAL: ++n_fatal_; break;
   }

   diagnostics_.emplace_back(level, location, std::move(message));
}

void This::push_info(SourceLocation location, string&& message) noexcept
{
   push_diagnostic_(Diagnostic::INFO, location, std::move(message));
}

void This::push_warn(SourceLocation location, string&& message) noexcept
{
   push_diagnostic_(Diagnostic::WARN, location, std::move(message));
}

void This::push_error(SourceLocation location, string&& message) noexcept
{
   push_diagnostic_(Diagnostic::ERROR, location, std::move(message));
}

void This::push_fatal(SourceLocation location, string&& message) noexcept
{
   push_diagnostic_(Diagnostic::FATAL, location, std::move(message));
}

void This::push_info(string&& message) noexcept
{
   push_info(scanner().current().location(), std::move(message));
}

void This::push_warn(string&& message) noexcept
{
   push_warn(scanner().current().location(), std::move(message));
}

void This::push_error(string&& message) noexcept
{
   push_error(scanner().current().location(), std::move(message));
}

void This::push_fatal(string&& message) noexcept
{
   push_fatal(scanner().current().location(), std::move(message));
}

// ------------------------------------------------------------------------ text

string_view text(const CompilerContext& context, const Token& token) noexcept
{
   assert(token.key() < context.texts().size());
   const auto& ss = context.texts().at(token.key());
   assert(token.offset() <= ss.size());
   assert(token.offset() + token.length() <= ss.size());
   return string_view(&ss[token.offset()], token.length());
}

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
                texts().size(),
                n_infos_,
                n_warns_,
                n_errors_,
                n_fatal_);

   for(const auto& diagnostic : diagnostics()) diagnostic.stream(ss, *this);

   return ss;
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

   // LL (i.e., parse) rules, we're not doing this...
   // WE ARE NOT DOING THIS... it's a procedure that
   // calculate_ll_rules(context, ast.get());

   // Print diagnostics
   std::for_each(
       cbegin(context.diagnostics()),
       cend(context.diagnostics()),
       [&](const auto& diagnostic) { diagnostic.stream(cout, context); });

   if(context.n_fatal() > 0 || context.n_errors() > 0) {
      return false; // we failed!
   }

   // Output some nice friendly stuff
   cout << '\n';
   ast->scope()->stream(cout);
   ast->stream(cout, context.curried_text_function());

   for(AstNode* r_node : *ast) {
      auto rule = cast_ast_node<RuleNode>(r_node);
      auto print_set
          = [&](auto& set) { return implode(cbegin(set), cend(set), ", "); };
      cout << format("{}:\n", text(context, rule->identifier()));
      cout << format("   first Set:  {}\n", print_set(rule->first_set()));
      cout << format("   final Set:  {}\n", print_set(rule->final_set()));
      cout << format("   follow Set: {}\n", print_set(rule->follow_set()));
      cout << endl;
   }
   return true; // we're winners
}

} // namespace giraffe

#undef This
