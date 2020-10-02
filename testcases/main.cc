
#ifdef CATCH_BUILD

// We know that `main.cc` is going to be first in unity builds.
// Therefore, we include our precompiled header here, so that it
// is first in the unity (testcases) build.
#include "stdinc.hpp"

// ---- Compile in catch
#define CATCH_CONFIG_PREFIX_ALL
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

// ------------------------------------------------------------------------ main

int main(int argc, char** argv)
{
   Catch::Session session; // There must be exactly one instance

   // Let Catch (using Clara) parse the command line
   auto return_code = session.applyCommandLine(argc, argv);
   if(return_code != EXIT_SUCCESS) return return_code; // Command line error

   return session.run();
}

#endif
