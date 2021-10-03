#!/bin/bash

old_ifs="$IFS"
IFS=,
last_algo=

rm --force --recursive plot/data plot/scripts plot/png
mkdir --parents plot/data plot/scripts

for compiler in g++ clang++ icpc icpx
do
    tail --lines +2 "$compiler.csv" \
        | sed 's/\//,/;s/"//g' \
        | while read -r algo size _ cpu_time _
    do
        output="plot/data/$compiler-$algo.plot"

        if [[ "$algo" != "$last_algo" ]]
        then
            true > "$output"

            gnuplot_file="plot/scripts/$algo.gp"

            if [ ! -s "$gnuplot_file" ]
            then
                algo_name="${algo//benchmark_/}"
                echo 'set title "find\\\_'"$algo_name"'"' > "$gnuplot_file"
                plot_command="plot"
            else
                plot_command=", \\"$'\n'"     "
            fi

            relative_output="${output#plot/}"
            echo -n "$plot_command \"$relative_output\" title \"$compiler\" with linespoints ps 2" \
                 >> "$gnuplot_file"

            last_algo="$algo"
        fi

        echo "$size $cpu_time" >> "$output"
    done

done

IFS="$old_ifs"

cd plot
gnuplot "per-algo.gp"
