set datafile separator "|"
set terminal pdf monochrome font "Helvetica,10" size 6,4

set xlabel "Expression size (tokens)"
set ylabel "Time (nanoseconds)"
set key off

# --- mode-specific helpers ---
unset output
set xrange [*:*]    # let gnuplot pick
set yrange [*:*]

# ---- Mode 1 ------------------------------------------------------------
set output "mode1.pdf"
plot "results" u ($1==1?$2:1/0):3 w points pt 7

# ---- Mode 2 ------------------------------------------------------------
set output "mode2.pdf"
plot "results" u ($1==2?$2:1/0):3 w points pt 5

# ---- Mode 3 ------------------------------------------------------------
set output "mode3.pdf"
plot "results" u ($1==3?$2:1/0):3 w points pt 9

# ---- Combined ----------------------------------------------------------
set output "performance.pdf"
set key bottom right
plot \
  "results" u ($1==1?$2:1/0):3 w points pt 7 t "Mode 1", \
  "results" u ($1==2?$2:1/0):3 w points pt 5 t "Mode 2", \
  "results" u ($1==3?$2:1/0):3 w points pt 9 t "Mode 3"

unset output
