#!/bin/bash

set -e

show_help()
{
    cat <<EOF
   Usage: $(basename 0) <toolchain>

   Where:

      <toolchain>        Is 'gcc' or 'clang'

   Example:

      # Set environment variables for clang 11.0.0
      > source $(basename $0) llvm-11.0.0

EOF
}

# Look for help
for ARG in "$@" ; do
    [ "$ARG" = "--help" ] || [ "$ARG" = "-h" ] && show_help && exit 0
done

[ "$#" != "1" ] && echo "Expected <toolchain>, aborting" && exit 0

TOOLCHAIN="$1"

OPT_ROOT="/opt/cc"
ARCH_ROOT="/opt/arch"

setup_build_env()
{
    local TOOLCHAIN="$1"

    local TOOL="$(echo "$TOOLCHAIN" | sed 's,-, ,' | awk '{ print $1 }')"
    local TC_VERSION="$(echo "$TOOLCHAIN" | sed 's,-, ,' | awk '{ print $2 }')"
    
    local TC_PATH="$TOOLSET"
    [ "$TOOLSET" = "clang" ] && TC_PATH="llvm"

    local CC_ROOT="$OPT_ROOT/$(echo "$TOOLCHAIN" | sed 's,llvm,clang,')"
    local ARCH_ROOT="$ARCH_ROOT/$TOOLCHAIN"
    
    export PREFIX="$ARCH_ROOT"
    export TC_CXXFLAGS="-isystem$PREFIX/include"
    export TC_LTO=
    export TC_TOOLSET="$TOOL"

    export TC_MODULES="-fmodules -fmodule-map-file=$CC_ROOT/include/c++/v1/module.modulemap"
    
    if [ "$TOOL" = "gcc" ] ; then

        export TC_TOOLSET="gcc"
        export TC_CC="$CC_ROOT/bin/gcc"
        export TC_CXX="$CC_ROOT/bin/g+"
        export TC_CXXFLAGS="-fPIC"
        export TC_LDFLAGS="-lm -lstdc++"
        export TC_ASAN_LINK="-fsanitize=address -lasan"
        export TC_USAN_LINK="-fsanitize=undefined -lubsan"           
        export TC_TSAN_LINK="-fsanitize=thread -fPIE"
        export TC_LTO_FLAG="-flto"
        export TC_LTO_LINK="-flto"
        export TC_MAX_ERROR="-fmax-errors"
        export TC_ARCHIVER="$CC_ROOT/bin/gcc-ar"
        export TC_RANDLIB="$CC_ROOT/bin/gcc-ranlib"
        export TC_WARN="-Wall -Wextra -Wpedantic -Winvalid-pch -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wno-unused-parameter -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wno-c++98-compat -Wchar-subscripts -Wformat -Wnull-dereference -Wmissing-include-dirs -Wuninitialized"
        export TC_C_WARN="-Wall -Wextra -Wpedantic -Winvalid-pch -Wcast-align -Wno-unused-parameter -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-member-function -Wformat-nonliteral -Wno-reserved-id-macro -Wno-format-nonliteral -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-shadow -Wno-documentation -Wno-sign-conversion -Wno-conversion -Wno-cast-align -Wno-double-promotion"
        
    elif [ "$TOOL" = "clang" ] || [ "$TOOL" = "llvm" ] ; then

        export TC_TOOLSET="clang"
        export TC_CC="$CC_ROOT/bin/clang"
        export TC_CXX="$CC_ROOT/bin/clang++"

        if [ "$TOOL" = "clang" ] ; then        
            export TC_CXXFLAGS="-fPIC -isystem$PREFIX/include"
            export TC_LDFLAGS="-fuse-ld=lld -lm -lstdc++ -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
            export TC_STATIC="-fuse-ld=lld -lm -lstdc++ -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"            
            export TC_ASAN_LINK="-fsanitize=address"
            export TC_USAN_LINK="-fsanitize=undefined -lubsan"           
            export TC_TSAN_LINK="-fsanitize=thread -fPIE"
            
        else
            export TC_CXXFLAGS="-fPIC -nostdinc++ -isystem$CC_ROOT/include/c++/v1 -isystem$CC_ROOT/include -isystem$PREFIX/include"
            export TC_LDFLAGS="-fuse-ld=lld -stdlib=libc++ -lm -L$CC_ROOT/lib -Wl,-rpath,$CC_ROOT/lib -lc++ -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
            export TC_STATIC="-fuse-ld=lld -stdlib=libc++ -lm -Wl,-Bstatic -L$CC_ROOT/lib -lc++ -lc++abi -lunwind -Wl,-Bdynamic -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
            export TC_ASAN_LINK0="-fsanitize=address -L$CC_ROOT/lib/clang/${TC_VERSION}/lib/linux -Wl,-rpath,$CC_ROOT/lib/clang/${TC_VERSION}/lib/linux -lclang_rt.asan-x86_64"
            export TC_ASAN_LINK="$CC_ROOT/lib/clang/${TC_VERSION}/lib/linux/libclang_rt.asan-x86_64.a -fsanitize=address"
            export TC_USAN_LINK="$CC_ROOT/lib/clang/${TC_VERSION}/lib/linux/libclang_rt.ubsan_standalone_cxx-x86_64.a -fsanitize=undefined"           
            export TC_TSAN_LINK="$CC_ROOT/lib/clang/${TC_VERSION}/lib/linux/libclang_rt.tsan_cxx-x86_64.a -fsanitize=thread -fPIE"

        fi
        export TC_LTO_FLAG="-flto=thin"
        export TC_LTO_LINK="-flto=thin"
        export TC_MAX_ERROR="-ferror-limit"
        export TC_ARCHIVER="$CC_ROOT/bin/llvm-ar"
        export TC_RANDLIB="$CC_ROOT/bin/llvm-ranlib"
        export TC_WARN="-Wall -Wextra -Wpedantic -Winvalid-pch -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wno-unused-parameter -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wno-c++98-compat -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-member-function -Wformat-nonliteral -Wno-reserved-id-macro -Wno-format-nonliteral -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-shadow -Wno-documentation -Wno-unused-variable -Wno-unused-private-field"
        export TC_C_WARN="-Wall -Wextra -Wpedantic -Winvalid-pch -Wcast-align -Wno-unused-parameter -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-member-function -Wformat-nonliteral -Wno-reserved-id-macro -Wno-format-nonliteral -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-shadow -Wno-documentation -Wno-sign-conversion -Wno-conversion -Wno-cast-align -Wno-double-promotion"
        export ASAN_SYMBOLIZER_PATH=${CC_ROOT}/bin/llvm-symbolizer

    else
        echo "opps!" && exit 1
    fi
}

setup_build_env "$TOOLCHAIN"

mobius_rules()
{
    cat <<EOF
# Autogenerated with: "$(basename "${BASH_SOURCE[0]}") $TOOLCHAIN"

CC             = nice ionice -c3 ${TC_CC}
MOC            = moc
RCC            = rcc
cc_desc        = ${TC_TOOLSET}
cpp_inc        = ${TC_CXXFLAGS}
link_libcpp_so = ${TC_LDFLAGS}
link_libcpp_a  = ${TC_STATIC}
gdb_flags      = -g3 -gdwarf-2 -fno-omit-frame-pointer -fno-optimize-sibling-calls
w_flags        = ${TC_WARN}
c_w_flags      = ${TC_C_WARN}
d_flags        = -DDEBUG_BUILD
r_flags        = -DRELEASE_BUILD -DNDEBUG
o_flag         = -O3
f_flags        = -fPIC -fdiagnostics-color=always ${TC_MAX_ERROR}=4
s_flags        = -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
l_flags        = ${TC_LDFLAGS}
ranlib         = ${TC_RANDLIB}
ar             = ${TC_ARCHIVER}
lto_flags      = ${TC_LTO_FLAG}
lto_link       = ${TC_LTO_LINK}
asan_flags     = -g3 -gdwarf-2 -DADDRESS_SANITIZE -fsanitize=address
usan_flags     = -g3 -gdwarf-2 -DUNDEFINED_SANITIZE -fsanitize=undefined
tsan_flags     = -g3 -gdwarf-2 -DTHREAD_SANITIZE -fsanitize=thread -fPIE
prof_flags     = -DBENCHMARK_BUILD -fno-omit-frame-pointer -g
prof_link      = -lbenchmark

c_release_flags   = \$o_flag  \$f_flags \$r_flags \$c_w_flags
cpp_release_flags = \$o_flag \$cpp_inc \$f_flags \$r_flags \$w_flags
release_link      = \$l_flags

c_debug_flags     = -O0 \$f_flags \$d_flags \$gdb_flags \$c_w_flags
cpp_debug_flags   = -O0 \$cpp_inc \$f_flags \$d_flags \$gdb_flags \$w_flags
debug_link        = \$l_flags

c_asan_flags      = -O0  \$f_flags \$s_flags \$asan_flags \$c_w_flags
cpp_asan_flags    = -O0 \$cpp_inc \$f_flags \$s_flags \$asan_flags \$w_flags
asan_link         = \$l_flags ${TC_ASAN_LINK}

c_usan_flags      = -O0  \$f_flags \$s_flags \$usan_flags \$c_w_flags
cpp_usan_flags    = -O0 \$cpp_inc \$f_flags \$s_flags \$usan_flags \$w_flags
usan_link         = \$l_flags ${TC_USAN_LINK}

c_tsan_flags      = -O0  \$f_flags \$s_flags \$tsan_flags \$c_w_flags
cpp_tsan_flags    = -O0 \$cpp_inc \$f_flags \$s_flags \$tsan_flags \$w_flags
tsan_link         = \$l_flags ${TC_TSAN_LINK}

EOF
}

TOOLCHAIN_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/toolchains" ; pwd)"
OUT_FILE="$TOOLCHAIN_DIR/flags-${TOOLCHAIN}.ninja"

mobius_rules > $OUT_FILE


