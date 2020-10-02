
#pragma once

#include "diagnostic.hpp"
#include "scanner/token-producer.hpp"

namespace giraffe
{
struct Token;

struct CompilerOptions final
{
   TokenProducerOptions token_producer_options = {};
   bool w_error                                = false;
};

struct CompilerContext final
{
   uint32_t n_errors              = 0;
   uint32_t n_fatal               = 0;
   CompilerOptions options        = {};
   vector<Diagnostic> diagnostics = {};
};

void push_error(CompilerContext& context,
                SourceLocation location,
                string&& message) noexcept;

} // namespace giraffe
