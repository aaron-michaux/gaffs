#!/bin/bash

set -e

PPWD="$(cd "$(dirname "$0")"; pwd)"

# ------------------------------------------------------------ Parse Commandline

CLEAN=
VERBOSE=
CONFIG=asan
TARGET_FILE0=gaffs
TOOLCHAIN=clang-11.0.0
NO_BUILD=0
GDB=0
BUILD_ONLY=0
BUILD_TESTS=0
BENCHMARK=0
BUILD_EXAMPLES=0
LTO=0
UNITY=0
VALGRIND=0
DO_INSTALL=0

TARGET_FILE="$TARGET_FILE0"

while [ "$#" -gt "0" ] ; do
    # Compiler
    [ "$1" = "clang" ]     && TOOLCHAIN="clang-11.0.0" && shift && continue
    [ "$1" = "llvm" ]      && TOOLCHAIN="llvm-11.0.0" && shift && continue
    [ "$1" = "gcc" ]       && TOOLCHAIN="gcc-10.2.0" && shift && continue

    # Configuration
    [ "$1" = "debug" ]     && CONFIG=debug && shift && continue
    [ "$1" = "valgrind" ]  && CONFIG=debug && VALGRIND=1 && shift && continue
    [ "$1" = "gdb" ]       && CONFIG=debug && GDB=1 && shift && continue
    [ "$1" = "release" ]   && CONFIG=release && shift && continue
    [ "$1" = "asan" ]      && CONFIG=asan && shift && continue    # sanitizers
    [ "$1" = "usan" ]      && CONFIG=usan && shift && continue
    [ "$1" = "tsan" ]      && CONFIG=tsan && shift && continue
    
    # Other options
    [ "$1" = "clean" ]     && CLEAN="-t clean" && shift && continue
    [ "$1" = "verbose" ]   && FEEDBACK="-v" && shift && continue # verbose ninja
    [ "$1" = "nf" ]        && FEEDBACK="-q" && shift && continue # quiet ninja
    [ "$1" = "mobius" ]    && NO_BUILD="1" && shift && continue  # no ninja
    [ "$1" = "build" ]     && BUILD_ONLY="1" && shift && continue
    [ "$1" = "test" ]      && BUILD_TESTS="1" && BUILD_EXAMPLES=1 && shift && continue
    [ "$1" = "unity" ]     && UNITY=1 && shift && continue
    [ "$1" = "lto" ]       && LTO=1 && shift && continue
    [ "$1" = "no-lto" ]    && LTO=0 && shift && continue
    [ "$1" = "bench" ]     && BENCHMARK=1 && shift && continue
    [ "$1" = "examples" ]  && BUILD_EXAMPLES=1 && shift && continue

    [ "$1" = "install" ]   && DO_INSTALL=1 \
        && LTO=1 && UNITY=1 && CONFIG=release && shift && continue
    
    # Target
    [ "$1" = "build.ninja" ] && TARGET_FILE=build.ninja && shift && continue
    break
done

if [ "${TOOLCHAIN:0:5}" = "clang" ] ; then
    export TOOL=clang
    export TOOL_VERSION="${TOOLCHAIN:6}"
elif [ "${TOOLCHAIN:0:4}" = "llvm" ] ; then
    export TOOL=llvm
    export TOOL_VERSION="${TOOLCHAIN:5}"
elif [ "${TOOLCHAIN:0:3}" = "gcc" ] ; then
    export TOOL=gcc
    export TOOL_VERSION="${TOOLCHAIN:4}"
else
    echo "Unsupported toolchain: ${TOOLCHAIN}"
    exit 1
fi

if [ "$BENCHMARK" = "1" ] && [ "$BUILD_TESTS" = "1" ] ; then
    echo "Cannot benchmark and build tests at the same time."
    exit 1
fi

# ---------------------------------------------------------------------- Execute

export MOBIUSDIR="$PPWD/project-config/toolchain-configs"
"$MOBIUSDIR/set-env.sh" "$TOOLCHAIN"

UNIQUE_DIR="${TOOL}-${TOOL_VERSION}-${CONFIG}"
[ "$BUILD_TESTS" = "1" ] && UNIQUE_DIR="test-${UNIQUE_DIR}"
[ "$LTO" = "1" ] && UNIQUE_DIR="${UNIQUE_DIR}-lto"
[ "$BENCHMARK" = "1" ] && UNIQUE_DIR="bench-${UNIQUE_DIR}"
[ "$UNITY" = "1" ] && UNIQUE_DIR="unity-${UNIQUE_DIR}"

export TOOLCHAIN="$TOOLCHAIN"
export BUILDDIR="/tmp/build-$USER/$UNIQUE_DIR/$TARGET_FILE"
export TARGET="build/$UNIQUE_DIR/$TARGET_FILE"
export SRC_DIRECTORIES="src"

mkdir -p "$(dirname "$TARGET")"

export C_FLAGS="\$c_${CONFIG}_flags"
export CPP_FLAGS="\$cpp_${CONFIG}_flags"
export LINK_FLAGS="\$${CONFIG}_link"

if [ "$LTO" = "1" ] ; then
    export C_FLAGS="\$lto_flags $C_FLAGS"
    export CPP_FLAGS="\$lto_flags $CPP_FLAGS"
    export LINK_FLAGS="\$lto_link $LINK_FLAGS"
fi

if [ "$BUILD_TESTS" = "1" ] ; then
    export SRC_DIRECTORIES="${SRC_DIRECTORIES} testcases"
    export C_FLAGS="${C_FLAGS} -DCATCH_BUILD"
    export CPP_FLAGS="${CPP_FLAGS} -DCATCH_BUILD"
fi

if [ "$BUILD_EXAMPLES" = "1" ] ; then
    export SRC_DIRECTORIES="${SRC_DIRECTORIES} examples"
    export CPP_FLAGS="$CPP_FLAGS -Wno-unused-function "
fi

if [ "$BENCHMARK" = "1" ] ; then
    export SRC_DIRECTORIES="${SRC_DIRECTORIES} benchmark"
    export C_FLAGS="${C_FLAGS} -DBENCHMARK_BUILD"
    export CPP_FLAGS="${CPP_FLAGS} -DBENCHMARK_BUILD"
    export LINK_FLAGS="-lpthread -L/usr/local/lib -lbenchmark ${LINK_FLAGS}"
fi

# ---- Run Mobius
run_mobius()
{
    cat "$PPWD/project-config/build.mobius" | mobius -m \$moduledir -
}

if [ "$NO_BUILD" = "1" ] ; then
    run_mobius
    exit $?
fi

NINJA_FILE="$(dirname "$TARGET")/build.ninja"
# Ensure that we've got the latest build.ninja file
! run_mobius > "$NINJA_FILE" \
    && exit 1   


[ "$TARGET_FILE" = "build.ninja" ] && exit 0

# If doing a unity build, then change the target
NINJA_TARGET="$TARGET"
[ "$UNITY" = "1" ] && NINJA_TARGET="$TARGET.unity"
! ninja -f "$NINJA_FILE" -C "$PPWD" -j $(expr `nproc` \* 2) \
  $FEEDBACK $CLEAN $NINJA_TARGET \
    && exit 1

# If doing a unity build, then link back to the target
[ "$UNITY" = "1" ] && cp "$TARGET.unity" "$TARGET"

! [ "$CLEAN" = "" ] && exit 0

[ "$BUILD_ONLY" = "1" ] && exit 0



if [ "$DO_INSTALL" = "1" ] ; then
    # ---- Install gaffs if that's what we're doing
    sudo cp "$PPWD/$TARGET" /usr/local/bin/
    
elif [ "$TARGET_FILE" = "$TARGET_FILE0" ] ; then
    # ---- If we're building the executable (TARGET_FILE0), then run it
    
    export LSAN_OPTIONS="suppressions=$PPWD/project-config/lsan.supp"
    export ASAN_OPTIONS="protect_shadow_gap=0,detect_leaks=0"
    
    PRODUCT="$PPWD/$TARGET"
        
    if [ "$GDB" = "1" ] ; then        
        gdb -ex run -silent -return-child-result -statistics --args "$PRODUCT" "$@"
        exit $?
        
    elif [ "$VALGRIND" = "1" ] ; then        
        valgrind --tool=memcheck --leak-check=full --track-origins=yes --verbose --log-file=valgrind.log --gen-suppressions=all "$PRODUCT" "$@"
        exit $?

    else
        "$PRODUCT" "$@"
        exit $?
        
    fi
         
fi

 
