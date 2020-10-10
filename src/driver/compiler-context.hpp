
#pragma once

#include "compiler-options.hpp"
#include "diagnostic.hpp"
#include "scanner/scanner.hpp"

namespace giraffe
{
struct Token;
class Scope;

/**
 * The compiler context has the following details:
 * [1] Any command line options (compiler/scanner opts).
 * [2] The text data of all input buffers.
 * [3] The scanner which produces the token sequence.
 * [4] Any diagnostics generated anywhere in the compiler.
 */
class CompilerContext final
{
 private:
   ScannerOptions scanner_opts_   = {};
   CompilerOptions compiler_opts_ = {};

   vector<string> names_ = {}; // names of parsed texts (normally filenames)
   vector<string> texts_ = {}; // the texts being scanned

   Scanner scanner_ = {};

   uint32_t n_infos_  = 0;
   uint32_t n_warns_  = 0;
   uint32_t n_errors_ = 0;
   uint32_t n_fatal_  = 0;

   vector<Diagnostic> diagnostics_ = {};

   void push_diagnostic_(Diagnostic::Level, SourceLocation, string&&) noexcept;

   CompilerContext() = default;

 public:
   CompilerContext(const CompilerContext&) = delete;
   CompilerContext(CompilerContext&&)      = default;
   ~CompilerContext()                      = default;
   CompilerContext& operator=(const CompilerContext&) = delete;
   CompilerContext& operator=(CompilerContext&&) = default;

   //@{ Construction
   static unique_ptr<CompilerContext> make(string_view text,
                                           CompilerOptions opts);

   static unique_ptr<CompilerContext>
   make(vector<string>&& names, vector<string>&& texts, CompilerOptions opts);
   //@}

   //@{ Getters
   const auto& scanner_opts() const noexcept { return scanner_opts_; }
   const auto& compiler_opts() const noexcept { return compiler_opts_; }
   const auto& names() const noexcept { return names_; }
   const auto& texts() const noexcept { return texts_; }
   const auto& scanner() const noexcept { return scanner_; }
   auto& scanner() noexcept { return scanner_; }
   auto n_infos() const noexcept { return n_infos_; }
   auto n_warns() const noexcept { return n_warns_; }
   auto n_errors() const noexcept { return n_errors_; }
   auto n_fatal() const noexcept { return n_fatal_; }
   const auto& diagnostics() const noexcept { return diagnostics_; }
   //@}

   //@{ Text (of tokens)
   // So you want to get the text of a token...
   friend string_view text(const CompilerContext&, const Token& token) noexcept;

   /**
    * Curry the `text(context, token)` function
    */
   auto curried_text_function() const noexcept
   {
      return [this](const Token& token) { return text(*this, token); };
   }
   //@}

   //@{ Pushing Diagnostics
   void push_info(SourceLocation location, string&& message) noexcept;
   void push_warn(SourceLocation location, string&& message) noexcept;
   void push_error(SourceLocation location, string&& message) noexcept;
   void push_fatal(SourceLocation location, string&& message) noexcept;

   // Uses the location of the current token
   void push_info(string&& message) noexcept;
   void push_warn(string&& message) noexcept;
   void push_error(string&& message) noexcept;
   void push_fatal(string&& message) noexcept;
   //@}

   std::ostream& stream(std::ostream&) const noexcept;
};

bool execute(CompilerContext& context) noexcept;

} // namespace giraffe
