
#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>

#include <unistd.h>

#include "cli.hpp"

#include "cli-args.hpp"

#include "driver/compiler-context.hpp"

namespace giraffe
{
static string read_stream(auto&& in)
{
   in >> std::noskipws;
   std::istream_iterator<char> bb(in);
   std::istream_iterator<char> ee;
   return std::string(bb, ee);
}

static bool is_regular_file(string_view filename) noexcept
{
   try {
      auto p = std::filesystem::path(filename);
      return std::filesystem::is_regular_file(p);
   } catch(...) {}
   return false;
}

// ------------------------------------------------------------------- show-help

void show_help(char* argv0) noexcept
{
   cout << format(R"V0G0N(

   Usage: {} [OPTIONS...] <filename>

   Options:

      -start <rulename>           Use <rulename> as the starting rule.
                                  Otherwise the first rule is used. 

      -Werror                     Treat warnings as errors.

      -fno-color-diagnostics      Turn off color diagnostics.
      -fcolor-diagnostics-always  Color nven no terminal is attached.

)V0G0N",
                  basename(argv0));
}

// ---------------------------------------------------------- parse-command-line

CliConfig parse_command_line(int argc, char** argv) noexcept
{
   CliConfig config = {};

   // First look for the help switch...
   for(auto i = 1; i < argc; ++i) {
      string_view arg = argv[i];
      if(arg == "-h" || arg == "--help") {
         config.show_help = true;
         return config;
      }
   }

   // Parse arguments
   bool no_color_diagnostics     = false;
   bool color_diagnostics_always = false;
   for(auto i = 1; i < argc; ++i) {
      string_view arg = argv[i];
      try {
         if(arg == "-start")
            config.start_rule = cli::safe_arg_str(argc, argv, i);
         else if(arg == "-Werror")
            config.compiler_opts.w_error = true;
         else if(arg == "-fno-color-diagnostics")
            no_color_diagnostics = true;
         else if(arg == "-fcolor-diagnostics-always")
            color_diagnostics_always = true;
         else if(is_regular_file(arg))
            config.fnames.emplace_back(begin(arg), end(arg));
         else if(arg == "-") // i.e., stdin
            config.fnames.push_back("-"s);
         else {
            std::cerr << format("Unexpected command-line argument: '{}'\n",
                                arg);
            config.has_error = true;
         }
      } catch(std::exception& e) {
         std::cerr << format(
             "Exception at command-line argument '{}': {}\n", arg, e.what());
         config.has_error = true;
      }
   }

   // Handle color diagnostics
   config.compiler_opts.color_diagnostics = (isatty(1) != 0);
   if(no_color_diagnostics) config.compiler_opts.color_diagnostics = false;
   if(color_diagnostics_always) config.compiler_opts.color_diagnostics = true;

   if(config.fnames.empty()) {
      std::cerr << format("Must specify at least 1 input file.\n");
      config.has_error = true;
   }

   return config;
}

// ---------------------------------------------------------------- init context

static unique_ptr<CompilerContext>
init_compiler_context(const CliConfig& config) noexcept
{
   if(config.has_error) return nullptr;

   bool has_error = false;

   vector<string> fnames = config.fnames;
   vector<string> texts;
   texts.reserve(config.fnames.size());

   for(const auto& fname : config.fnames) {
      try {
         if(fname == "-")
            texts.push_back(read_stream(std::cin));
         else
            texts.push_back(
                read_stream(std::fstream(fname, std::ios_base::in)));
      } catch(std::exception& e) {
         std::cerr << format(
             "Exception while reading '{}': {}", fname, e.what())
                   << endl;
         has_error = true;
      }
   }

   return (has_error) ? nullptr
                      : CompilerContext::make(std::move(fnames),
                                              std::move(texts),
                                              config.compiler_opts);
}

// ------------------------------------------------------------------------- run

int run(int argc, char** argv) noexcept
{
   const auto config = parse_command_line(argc, argv);
   if(config.show_help) {
      show_help(argv[0]);
      return EXIT_SUCCESS;
   }

   auto context = init_compiler_context(config);
   if(context == nullptr) {
      cout << "Aborting due to previous errors." << endl;
      return EXIT_FAILURE;
   }

   const auto success = execute(*context);
   return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace giraffe
