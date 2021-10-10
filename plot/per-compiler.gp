#!/usr/bin/gnuplot -persist

set terminal pngcairo font "Sans,12" linewidth 3 size 1280,720

set size
set xlabel "Input size"
set ylabel "Relative processing time"
set yrange [0.2 : 1.2]

set key Right right

set logscale x

system "mkdir --parents png/".ARG1

set output "png/".ARG1."/relative-g++.png"
load "scripts/".ARG1."/g++.gp"

set output "png/".ARG1."/relative-clang++.png"
load "scripts/".ARG1."/clang++.gp"

set output "png/".ARG1."/relative-icpc.png"
load "scripts/".ARG1."/icpc.gp"

set output "png/".ARG1."/relative-icpx.png"
load "scripts/".ARG1."/icpx.gp"

#    EOF
