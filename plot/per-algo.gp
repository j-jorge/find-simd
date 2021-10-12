#!/usr/bin/gnuplot -persist

set terminal pngcairo font "Sans,12" linewidth 3 size 1280,720

set size
set xlabel "Input size"
set ylabel "Processing time, nanoseconds"
set yrange [0:12000000]
set key Left left reverse

#set logscale x
#set logscale y

system "mkdir --parents png/".ARG1

set output "png/".ARG1."/find_c.png"
load "scripts/".ARG1."/benchmark_c.gp"

set output "png/".ARG1."/find_c_unrolled_8.png"
load "scripts/".ARG1."/benchmark_c_unrolled_8.gp"

set output "png/".ARG1."/find_cpp.png"
load "scripts/".ARG1."/benchmark_cpp.gp"

set output "png/".ARG1."/find_sse2.png"
load "scripts/".ARG1."/benchmark_sse2.gp"

#    EOF
