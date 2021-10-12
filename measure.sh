#!/bin/bash

benchmarks=()
compilers=()

while [ $# -ne 0 ]
do
    arg="$1"
    shift

    if [ "$arg" = "--benchmark" ]
    then
        while [ $# -ne 0 ]
        do
            if [[ "$1" == --* ]]
            then
                break;
            else
                benchmarks+=("$1")
                shift
            fi
        done
    elif [ "$arg" = "--compiler" ]
    then
        while [ $# -ne 0 ]
        do
            if [[ "$1" == --* ]]
            then
                break;
            else
                compilers+=("$1")
                shift
            fi
        done
    fi
done

if [ "${#benchmarks[@]}" -eq 0 ]
then
    benchmarks=(last random)
fi

if [ "${#compilers[@]}" -eq 0 ]
then
    compilers=(clang++ g++ icpc icpx)
fi

intel_linux="/opt/intel/oneapi/compiler/latest/linux/"
intel_libraries="${intel_linux}/compiler/lib/intel64_lin/"

mkdir measures

for benchmark in "${benchmarks[@]}"
do
    for compiler in "${compilers[@]}"
    do
        echo "$compiler"
        LD_LIBRARY_PATH="${intel_libraries}:${LD_LIBRARY_PATH}" \
                       "./cmake-build/$compiler/benchmark-$benchmark" \
                       --benchmark_format=csv \
                       > "measures/$benchmark-$compiler.csv"
    done
done
