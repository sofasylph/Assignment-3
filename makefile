CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2
LDFLAGS=-lm

all: frpc measure

frpc: frpc.c commlib.a
	$(CC) $(CFLAGS) -o frpc frpc.c commlib.a $(LDFLAGS)

measure: measure.c
	$(CC) $(CFLAGS) -o measure measure.c
	./measure -i expressions.txt
	gnuplot plot_times.gp
	@echo \"Finished → results  errors  performance.pdf\"

graphs:
	gnuplot plot_all.gp
	@echo "Graphs → mode1.pdf  mode2.pdf  mode3.pdf  performance.pdf"

format:
	clang-format -i frpc.c measure.c commlib.c commlib.h

clean:
	rm -f frpc measure results errors performance.pdf mode1.pdf mode2.pdf mode3.pdf
