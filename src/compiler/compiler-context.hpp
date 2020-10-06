
#pragma once

#include "diagnostic.hpp"
#include "scanner/scanner.hpp"
//#include "symbol-table.hpp"

namespace giraffe
{
struct Token;

struct CompilerOptions final
{
   ScannerOptions scanner_options = {};
   bool color_diagnostics         = true;
   bool w_error                   = false;
};

class CompilerContext final
{
 public:
   CompilerOptions options = {};

   vector<string> names = {}; // names of parsed texts (normally filenames)
   vector<string> texts = {}; // the texts being scanned
   uint32_t n_infos     = 0;
   uint32_t n_warnings  = 0;
   uint32_t n_errors    = 0;
   uint32_t n_fatal     = 0;

   Scanner tokens = {};

   vector<Diagnostic> diagnostics = {};

   std::ostream& stream(std::ostream&) const noexcept;
};

// So you want to get the text of a token...
string_view text(const CompilerContext&, const Token& token) noexcept;

CompilerContext make_compiler_context(string_view text,
                                      CompilerOptions opts = {});

void push_error(CompilerContext& context,
                SourceLocation location,
                string&& message) noexcept;

// Uses the location of the current token
void push_error(CompilerContext& context, string&& message) noexcept;

void push_warn(CompilerContext& context,
               SourceLocation location,
               string&& message) noexcept;

// Uses the location of the current token
void push_warn(CompilerContext& context, string&& message) noexcept;

} // namespace giraffe
