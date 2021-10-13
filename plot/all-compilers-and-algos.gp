#!/usr/bin/gnuplot -persist

set terminal pngcairo font "Sans,12" linewidth 3 size 1280,720

set size
set xlabel "Input size"
set ylabel "Relative processing time"
set yrange [0.2 : 1.2]

set key outside Left right

set logscale x

system "mkdir --parents png/".ARG1

set output "png/".ARG1."/all.png"
load "scripts/".ARG1."/all.gp"

#    EOF
