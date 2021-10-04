#!/bin/bash

if [ $# -eq 0 ]
then
    tags=(clang++ g++ icpc icpx)
else
    tags=("$@")
fi

intel_linux="/opt/intel/oneapi/compiler/latest/linux/"
intel_libraries="${intel_linux}/compiler/lib/intel64_lin/"

for tag in "${tags[@]}"
do
    echo "$tag"
    LD_LIBRARY_PATH="${intel_libraries}:${LD_LIBRARY_PATH}" \
        "./cmake-build/$tag/run-benchmark" --benchmark_format=csv > "$tag.csv";
done
