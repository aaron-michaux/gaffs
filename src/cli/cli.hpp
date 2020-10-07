
#pragma once

#include "driver/compiler-options.hpp"

namespace giraffe
{
struct CliConfig final
{
   bool show_help                = false;
   bool has_error                = false;
   vector<string> fnames         = {};
   CompilerOptions compiler_opts = {};
};

CliConfig parse_command_line(int argc, char** argv) noexcept;
void show_help(char* argv0) noexcept;

int run(int argc, char** argv) noexcept;

} // namespace giraffe
