GCC=gcc
CFLAGS=-Wall -O2
exe=gaussian_parallel
objs=gaussian_parallel.o
main=gaussian_parallel.c

seq_gaus_exe=gaussian_sequential
seq_gaus_objs=gaussian_sequential.o
seq_gaus_main=gaussian_sequential.c

qsortseq_exe=qsortseq
qsortseq_objs=qsortseq.o
qsortseq_main=qsortseq.c

qsortpar_exe=qsortpar
qsortpar_objs=qsortpar.o
qsortpar_main=qsortpar.c

%.o: %.c
	$(GCC) $(CFLAGS) $< -c -o $@

$(exe): $(objs)
	$(GCC) $^ -o $@ -lpthread

compile: $(seq_gaus_exe) $(exe) $(qsortseq_exe) $(qsortpar_exe)
	echo "Built"

clean:
	rm -f $(exe) $(objs)
	rm -f $(seq_gaus_exe) $(seq_gaus_objs)
	rm -f $(qsortseq_exe) $(qsortseq_objs)
	rm -f $(qsortpar_exe) $(qsortpar_objs)