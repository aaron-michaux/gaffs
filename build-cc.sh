#!/bin/bash

set -e

sudo echo "Got root permissions"

OPT_ROOT=/opt/cc

NO_CLEANUP=1
if [ "$NO_CLEANUP" = "1" ] ; then
    TMPD=/tmp/build-cc
    mkdir -p $TMPD
else
    TMPD=$(mktemp -d /tmp/$(basename $0).XXXXXX)
fi

trap cleanup EXIT
cleanup()
{
    if ! [ "$NO_CLEANUP" = "1" ] ; then
        rm -rf $TMPD
    fi    
}

# ------------------------------------------------ ensure dependencies installed

sudo apt-get install -y \
     wget subversion automake swig python2.7-dev libedit-dev libncurses5-dev \
     python3-dev python3-pip python3-tk python3-lxml python3-six \
     libparted-dev flex sphinx-doc guile-2.2 gperf gettext expect tcl dejagnu \
     libgmp-dev libmpfr-dev libmpc-dev libasan6

# ------------------------------------------------------------------------ clang

build_clang()
{
    local CLANG_V="$1"
    local TAG="$2"
    local LLVM_DIR="llvm"

    local SRC_D=$TMPD/$LLVM_DIR
    local BUILD_D="$TMPD/build-llvm-${TAG}"

    local INSTALL_PREFIX="${OPT_ROOT}/clang-${TAG}"
    
    rm -rf "$BUILD_D"
    mkdir -p "$SRC_D"
    mkdir -p "$BUILD_D"

    cd "$SRC_D"

    ! [ -d "llvm-project" ] &&
        git clone https://github.com/llvm/llvm-project.git
    cd llvm-project
    
    git checkout "$CLANG_V"

    cd "$BUILD_D"

    nice ionice -c3 cmake -G "Ninja" \
         -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;libcxx;libcxxabi;libunwind;compiler-rt;lld;polly;lldb" \
         -DCMAKE_BUILD_TYPE=release \
         -DCMAKE_C_COMPILER=clang-6.0 \
         -DCMAKE_CXX_COMPILER=clang++-6.0 \
         -DLLVM_ENABLE_ASSERTIONS=Off \
         -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=Yes \
         -DLIBCXX_ENABLE_SHARED=YES \
         -DLIBCXX_ENABLE_STATIC=YES \
         -DLIBCXX_ENABLE_FILESYSTEM=YES \
         -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=YES \
         -LLVM_ENABLE_LTO=thin \
         -LLVM_USE_LINKER=lld-6.0 \
         -DLLVM_BUILD_LLVM_DYLIB=YES \
         -DPYTHON_EXECUTABLE=/usr/bin/python3.6m \
         -DPYTHON_LIBRARY=/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6m.so \
         -DPYTHON_INCLUDE_DIR=/usr/include/python3.6m \
         -DCURSES_LIBRARY=/usr/lib/x86_64-linux-gnu/libncurses.a \
         -DCURSES_INCLUDE_PATH=/usr/include/ \
         -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_PREFIX} \
         $SRC_D/llvm-project/llvm

    /usr/bin/time -v nice ionice -c3 ninja 2>$BUILD_D/stderr.text | tee $BUILD_D/stdout.text
    sudo ninja install | tee -a $BUILD_D/stdout.text
    cat $BUILD_D/stderr.text   
}

# -------------------------------------------------------------------------- gcc

build_gcc()
{
    TAG="$1"
    SUFFIX="$1"
    if [ "$2" != "" ] ; then SUFFIX="$2" ; fi

    SRCD="$TMPD/$SUFFIX"
    mkdir -p "$SRCD"
    cd "$SRCD"
    [ ! -d "gcc" ] && \
        git clone -b releases/gcc-${TAG} https://github.com/gcc-mirror/gcc.git

    if [ -d "$SRCD/build" ] ; then rm -rf "$SRCD/build" ; fi
    mkdir -p "$SRCD/build"
    cd "$SRCD/build"
    
    nice ionice -c3 ../gcc/configure --prefix=${OPT_ROOT}/gcc-${SUFFIX} --enable-languages=all --disable-multilib 
    /usr/bin/time -v nice ionice -c3 make -j$(nproc) 2>$SRCD/build/stderr.text | tee $SRCD/build/stdour.text
    sudo make install | tee -a $SRCD/build/stdour.text
}

# --------------------------------------------------------------------- valgrind

build_valgrind()
{
    VALGRIND_VERSION="$1"
    URL="http://www.valgrind.org/downloads/valgrind-${VALGRIND_VERSION}.tar.bz2"
    VAL_D="$TMPD/valgrind-${VALGRIND_VERSION}"
    rm -rf "$VAL_D"
    cd "$TMPD"
    wget "$URL"
    bzip2 -dc valgrind-${VALGRIND_VERSION}.tar.bz2 | tar -xf -
    rm -f valgrind-${VALGRIND_VERSION}.tar.bz2
    cd "$VAL_D"
    export CC=clang-6.0
    ./autogen.sh
    ./configure --prefix=/usr/local
    nice ionice -c3 make -j$(nproc)
    sudo make install
}


# ------------------------------------------------------------------------ build

build_clang llvmorg-11.0.0 11.0.0
build_gcc 10.2.0
# build_valgrind 3.14.0

