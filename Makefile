GCC=gcc
CFLAGS=-O2

gaussian_parallel_exe=gaussian_parallel.a
gaussian_parallel_objs=gaussian_parallel.o
gaussian_parallel_main=gaussian_parallel.c

seq_gaus_exe=gaussian_sequential.a
seq_gaus_objs=gaussian_sequential.o
seq_gaus_main=gaussian_sequential.c

qsortseq_exe=qsortseq.a
qsortseq_objs=qsortseq.o
qsortseq_main=qsortseq.c

qsortpar_exe=qsortpar.a
qsortpar_objs=qsortpar.o
qsortpar_main=qsortpar.c

compile_all: $(seq_gaus_exe) $(gaussian_parallel_exe) $(qsortseq_exe) $(qsortpar_exe)

%.o: %.c
	$(GCC) $(CFLAGS) $< -c -o $@

$(gaussian_parallel_exe): $(gaussian_parallel_objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^

$(seq_gaus_exe): $(seq_gaus_objs)
	$(GCC) $(CFLAGS) -o $@ $^
	
$(qsortpar_exe): $(qsortpar_objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^

$(qsortseq_exe): $(qsortseq_objs)
	$(GCC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(gaussian_parallel_exe) $(gaussian_parallel_objs)
	rm -f $(seq_gaus_exe) $(seq_gaus_objs)
	rm -f $(qsortseq_exe) $(qsortseq_objs)
	rm -f $(qsortpar_exe) $(qsortpar_objs)