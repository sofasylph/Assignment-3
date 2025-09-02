set terminal pdf monochrome font "Helvetica,10" size 6,4
set output "performance.pdf"

set datafile separator "|"

set xlabel "Expression size (tokens)"
set ylabel "Time (nanoseconds)"
set key bottom right

plot \
  "results" using ($1==1?$2:1/0):3 title "Mode 1" with linespoints pt 7, \
  "results" using ($1==2?$2:1/0):3 title "Mode 2" with linespoints pt 5, \
  "results" using ($1==3?$2:1/0):3 title "Mode 3" with linespoints pt 9
