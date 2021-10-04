#!/bin/bash

set -e

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

build()
{
    local tag="$1"
    local cmake_dir

    cmake_dir="$(pwd)/cmake-build/$tag"

    if [ ! -f "${cmake_dir}/CMakeCache.txt" ]
    then
        mkdir --parents "$cmake_dir"

        cd "$cmake_dir"
        cmake "${script_dir}" -DCMAKE_BUILD_TYPE=Release -G Ninja
    fi

    cmake --build "${cmake_dir}"
}

if [ $# -eq 0 ]
then
    pids=()

    "${BASH_SOURCE[0]}" g++ &
    pids+=($!)

    "${BASH_SOURCE[0]}" clang++ &
    pids+=($!)

    "${BASH_SOURCE[0]}" icpc &
    pids+=($!)

    "${BASH_SOURCE[0]}" icpx &
    pids+=($!)

    for pid in "${pids[@]}"
    do
        wait "$pid"
    done

    exit 0
fi

intel_linux="/opt/intel/oneapi/compiler/latest/linux/"

case "$1" in
     g++)
         CXX=g++ build g++
         ;;
     clang++)
         CXX=clang++ build clang++
         ;;
     icpc)
         CXX="${intel_linux}/bin/intel64/icpc" build icpc
         ;;
     icpx)
         LD_LIBRARY_PATH="${intel_linux}/compiler/lib/intel64_lin/:${LD_LIBRARY_PATH}" \
             CXX="${intel_linux}/bin/icpx" \
             build icpx
         ;;
esac
