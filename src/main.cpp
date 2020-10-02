
// We know that `main.cpp` is going to be first in unity builds.
// Therefore, we include our precompiled header here, so that it
// is first in the unity (testcases) build.
#include "stdinc.hpp"

// Don't compile in main(...) if we're doing a testcase build
#ifndef CATCH_BUILD

#include <cstdio>

int main(int, char**)
{
   printf("Hello, World!\n");
   return 0;
}

#endif
