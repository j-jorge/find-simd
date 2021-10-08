#!/bin/bash

generate_plot()
{
    local benchmark="$1"

    rm --force --recursive \
       "plot/data/$benchmark" \
       "plot/scripts/$benchmark" \
       "plot/png/$benchmark"
    mkdir --parents \
       "plot/data/$benchmark" \
       "plot/scripts/$benchmark" \
       "plot/png/$benchmark"

    old_ifs="$IFS"
    IFS=,
    last_algo=

    for compiler in g++ clang++ icpc icpx
    do
        tail --lines +2 "$benchmark-$compiler.csv" \
            | sed 's/\//,/;s/"//g' \
            | while read -r algo size _ cpu_time _
        do
            output="plot/data/$benchmark/$compiler-$algo.plot"

            if [[ "$algo" != "$last_algo" ]]
            then
                true > "$output"

                gnuplot_file="plot/scripts/$benchmark/$algo.gp"

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
    gnuplot -c "per-algo.gp" "$benchmark"
    cd - > /dev/null
}

for benchmark in last random
do
    if [ -f "$benchmark-g++.csv" ]
    then
        generate_plot "$benchmark"
    fi
done
