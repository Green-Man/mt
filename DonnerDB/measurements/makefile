CC=g++
LD=g++

CCFLAGS = -O3
CCFLAGS = -g

all: plot_wax plot_milk

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@

plot_wax.o: plot.cc
	$(CC) $(CCFLAGS) -DWAX -c $< -o $@	

plot_milk.o: plot.cc
	$(CC) $(CCFLAGS) -c $< -o $@	

plot_wax: plot_wax.o pfm.o
	$(LD) -o plot_wax plot_wax.o pfm.o

plot_milk: plot_milk.o pfm.o
	$(LD) -o plot_milk plot_milk.o pfm.o

clean:
	/bin/rm -f *.o *~ \#*\# plot_wax plot_milk

