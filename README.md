 
# Grammar Analysis of First Follow Sets

Gaffs is a tool for analyzing context free grammars.

## Building

A `build.ninja` file is supplied that build `gaffs` asan mode. 
Tested on clang-11. Edit the top two lines of `build.ninja` to
point it at your own installation.


``` bash

 # Edit the top two lines of 'build.ninja' to point it at your 
 # Clang 11 installation. (See below if you don't have clang installed.)

 > nano build.ninja
 > ninja
 > ./gaffs examples/gaffs.g

```

### If you don't have clang

``` bash

 # Downloads, compiles, installs clang-11 and gcc-10.2, installing
 # to '/opt/cc'. Tested on Ubuntu 18.04. YMMV. (Basically update the
 # dependent pacakges for your distribution.)
 
 > sudo ./build-cc.sh

```

### What's this run.sh thingy?

It compiles and runs `gaffs` using `gcc` or `clang`, and building `release`, `debug`, `asan`, `usan`, and `tsan`. I find it convenient. If you want to
get it running, you need to download and install `mobius`, which is a 
preprocessor for ninja-build.

``` bash

 # Install 'mobius'
 > git clone git@github.com:aaron-michaux/mobius-legacy.git
 > cd mobius-legacy
 > make
 > sudo make install
 
 # Install gcc and clang
 > cd /path/to/gaffs/project
 > sudo ./build-cc.sh
 
 # Enjoy! eg, ./run.sh [gcc|clang|llvm] unity? [debug|release|gdb|asan|usan|tsan] ...
 > ./run.sh gcc asan examples/gaffs.g
 
 # llvm uses clang and libc++. 'lto' turns on link-to-optimization.
 > ./run.sh llvm release lto examples/gaffs.g
 
 # You can do a unity build! Yes, I did find ODR violations using this.
 > ./run.sh clang unity debug examples/gaffs.g

```

## Grammar

The input file format is in `gaff` format, which is inspired by EBNF, but more "normal" if you're used to regular expressions. A `gaff` parser for the `gaff` format is given at the end of this section.

```
// Tokens are "ALL-CAPs", or strings

Grammar: Rule* EOF ;

Rule: IDENTIFIER ':' ElementList ('|' ElementList)* ';' ;

ElementList: (Element ElementSuffix?)+ ;

Element: IDENTIFIER | STRING | '(' ElementList ')' ;

ElementSuffix: '*' | '+' | '?' ;
```

