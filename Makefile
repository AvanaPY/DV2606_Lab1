GCC=gcc
CFLAGS=-O2

exe=gaussian_parallel.a
objs=gaussian_parallel.o
main=gaussian_parallel.c

seq_gaus_exe=gaussian_sequential.a
seq_gaus_objs=gaussian_sequential.o
seq_gaus_main=gaussian_sequential.c

qsortseq_exe=qsortseq.a
qsortseq_objs=qsortseq.o
qsortseq_main=qsortseq.c

qsortpar_exe=qsortpar.a
qsortpar_objs=qsortpar.o
qsortpar_main=qsortpar.c

compile_all: $(seq_gaus_exe) $(exe) $(qsortseq_exe) $(qsortpar_exe)

%.o: %.c
	$(GCC) $(CFLAGS) $< -c -o $@

$(exe): $(objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^

$(seq_gaus_exe): $(seq_gaus_objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^
	
$(qsortseq_exe): $(qsortseq_objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^
	
$(qsortpar_exe): $(qsortpar_objs)
	$(GCC) $(CFLAGS) -pthread -o $@ $^

clean:
	rm -f $(exe) $(objs)
	rm -f $(seq_gaus_exe) $(seq_gaus_objs)
	rm -f $(qsortseq_exe) $(qsortseq_objs)
	rm -f $(qsortpar_exe) $(qsortpar_objs)