#!/bin/bash

set -e

compilers=(g++ clang++ icpc icpx)

generate_plot_per_algo()
{
    local benchmark="$1"

    old_ifs="$IFS"
    IFS=,
    last_algo=

    for compiler in "${compilers[@]}"
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
                    algo_name="${algo_name//_/\\\\_}"
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
}

generate_plot_one_compiler()
{
    local benchmark="$1"
    local compiler="$2"

    tail --lines +2 "$benchmark-$compiler.csv" \
        | sed 's/^[^_]\+_//;s/\//,/;s/"//g' \
        | awk -F, '
{
  algo=$1;
  size=$2;
  cpu_time=$5;

  sizes[size]=1;
  algos[algo]=1;
  t[algo "/" size] = cpu_time;
}
END {
  i=0;

  for (k in sizes)
  {
    s[i] = int(k);
    ++i;
  }

  n=asort(s);
  i=0;

  for (k in algos)
  {
    algos_sorted[i] = k;
    ++i;
  }

  asort(algos_sorted);

  printf("#input_size");

  for (a in algos_sorted)
    printf("\t%s", algos_sorted[a]);

  printf("\n");

  for (i=1; i<=n; ++i)
  {
    size=s[i];
    printf("%s", size);

    for (a in algos_sorted)
    {
      printf("\t%s", t[algos_sorted[a] "/" size]);
    }
    printf("\n");
  }
}' > "plot/data/$benchmark/$compiler.plot"
}

generate_gnuplot_one_compiler()
{
    local benchmark="$1"
    local compiler="$2"
    local gnuplot_file="plot/scripts/$benchmark/$compiler.gp"
    local plot_file="plot/data/$benchmark/$compiler.plot"
    local relative_plot_file="${plot_file#plot/}"

    local plot_command="plot"
    local column=2

    echo "set title \"Relative performance ($compiler)\"" \
         > "$gnuplot_file"

    head --lines 1 "$plot_file" \
        | tr '\t' '\n' \
        | tail --lines +2 \
        | while read -r algo
    do
        local title

        if [[ "$algo" = "c" ]]
        then
            title="c (baseline)"
        else
            title="${algo//_/\\\\_/}"
        fi

        echo -n "$plot_command \"$relative_plot_file\" using 1:(\$$column/\$2) title \"$title\" with lines"
        plot_command=", \\"$'\n'"     "
        column=$((column+1))

    done >> "$gnuplot_file"
}

generate_plot_per_compiler()
{
    local benchmark="$1"

    for compiler in "${compilers[@]}"
    do
        generate_plot_one_compiler "$benchmark" "$compiler"
        generate_gnuplot_one_compiler "$benchmark" "$compiler"
    done
}

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

    generate_plot_per_algo "$benchmark"
    generate_plot_per_compiler "$benchmark"

    cd plot
    gnuplot -c "per-algo.gp" "$benchmark"
    gnuplot -c "per-compiler.gp" "$benchmark"
    cd - > /dev/null
}

for benchmark in last random
do
    if [ -f "$benchmark-g++.csv" ]
    then
        generate_plot "$benchmark"
    fi
done
