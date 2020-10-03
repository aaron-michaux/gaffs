
#pragma once

#include "diagnostic.hpp"
#include "scanner/scanner.hpp"

namespace giraffe
{
struct Token;

struct CompilerOptions final
{
   ScannerOptions scanner_options = {};
   bool color_diagnostics         = true;
   bool w_error                   = false;
};

struct CompilerContext final
{
   CompilerOptions options = {};

   vector<string> names = {}; // names of those texts (normally filenames)
   vector<string> texts = {}; // the texts being scanned
   uint32_t n_infos     = 0;
   uint32_t n_warnings  = 0;
   uint32_t n_errors    = 0;
   uint32_t n_fatal     = 0;

   Scanner tokens                 = {};
   vector<Diagnostic> diagnostics = {};

   std::ostream& stream(std::ostream&) const noexcept;
};

CompilerContext make_compiler_context(string_view text,
                                      CompilerOptions opts = {});

void push_error(CompilerContext& context,
                SourceLocation location,
                string&& message) noexcept;

// Uses the location of the current token
void push_error(CompilerContext& context, string&& message) noexcept;

} // namespace giraffe
