
#pragma once

#include <string>
#include <string_view>
#include <vector>

/**
 * @defgroup cli Command Line Utils
 * @ingroup giraffe-utils
 *
 * The `giraffe` method for parsing command-line arguments.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}

#include "cli/cli-args.hpp"

namespace giraffe
{
struct Config
{
   bool show_help{false};
   string filename{""};
};

int run_main(int argc, char** argv)
{
   Config config;
   auto has_error = false;

   for(int i = 1; i < argc; ++i) {
      string arg = argv[i];
      try {
         if(arg == "-h" || arg == "--help") {
            config.show_help = true;
         } else if(arg == "-f") {
            config.filename = cli::safe_arg_str(argc, argv, i);
         } else {
            cout << format("unexpected argument: '{}'", arg) << endl;
            has_error = true;
         }

      } catch(std::runtime_error& e) {
         cout << format("Error on command-line: {}", e.what()) << endl;
         has_error = true;
      }
   }

   if(has_error) {
      cout << format("aborting...") << endl;
      return EXIT_FAILURE;
   }

   // ---

   return EXIT_SUCCESS;
}

} // namespace giraffe
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */

namespace giraffe::cli
{
std::string safe_arg_str(int argc, char** argv, int& i) noexcept(false);
int safe_arg_int(int argc, char** argv, int& i) noexcept(false);
double safe_arg_double(int argc, char** argv, int& i) noexcept(false);

// ------------------------------------------------------------------ parse args

std::vector<std::string>
parse_cmd_args(const std::string_view line) noexcept(false);

} // namespace giraffe::cli
