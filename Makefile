GCC=gcc
CFLAGS=-Wall -O2
exe=parallel
main=gaussian_parallel.c
objs=gaussian_parallel.o

%.o: %.c
	$(GCC) $(CFLAGS) $< -c -o $@

$(exe): $(objs)
	$(GCC) $^ -o $@ -lpthread

run: $(exe)
	./$(exe) -n 2048
	
timeit: $(exe)
	time ./$(exe) -n 2048

clean:
	rm -f parallel
	rm -f $(objs)
	