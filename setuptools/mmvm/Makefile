include ../Makefile.inc

TARGET = mmvm
CXX    = g++
CXXFLAGS = --std=c++11 -Wall -O2

8086CC = 8086v6-cc
8086OPTS = -.o
ASMTEST = tests/assem
CTEST = tests/c
FORKTEST = $(CTEST)/forkexec
MINIX_TEST = tests/minix_binary
M2CC = m2cc

M2CMDS = bin/cc bin/nm bin/strip bin/ar bin/crc \
		 lib/as lib/ld lib/cv

OBJS = binary.o main.o x86disassm.o x86dump.o dump.o v6dump.o x86runtime.o unixv6.o x86unixv6.o \
minix.o x86minix.o os.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

install: $(TARGET)
	mkdir -p $(LOCAL)/bin
	mkdir -p $(PREFIX)/bin
	install -cs $(TARGET) $(PREFIX)/bin
	ln -s $(PREFIX)/bin/$(TARGET) $(LOCAL)/bin/$(TARGET)
	for cmd in $(M2CMDS); do \
		sh ./wrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 usr/$$cmd; \
	done

uninstall:
	rm -rf $(PREFIX)/bin/*
	rm $(LOCAL)/bin/$(TARGET)

test: $(ASMTEST)/write.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target

test1: $(ASMTEST)/write-1.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target1

test2: $(ASMTEST)/write-2.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target2

test3: $(ASMTEST)/write-3.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target3

test4: $(ASMTEST)/write-4.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target4

test5: $(ASMTEST)/write-5.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target5

test6: $(ASMTEST)/write-6.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target6

test7: $(ASMTEST)/write-7.s
	$(8086CC) $(8086OPTS) $^
	mv a.out target7

testc1: $(CTEST)/1.c
	$(8086CC) $^
	mv a.out c1

testc2: $(CTEST)/2.c
	$(8086CC) $^
	mv a.out c2

testc3: $(CTEST)/3.c
	$(8086CC) $^
	mv a.out c3

testc4: $(CTEST)/4.c
	$(8086CC) $^
	mv a.out c4

testc5: $(CTEST)/5.c
	$(8086CC) $^
	mv a.out c5

testc6: $(CTEST)/6.c
	$(8086CC) $^
	mv a.out c6

extest: extest.o
	$(CXX) -o $@ $^

cctest:
	7run -d $(MINIX_TEST)/cc > 7run_cc
	./mmvm -d $(MINIX_TEST)/cc > my_cc
	diff 7run_cc my_cc
kntest:
	7run -d $(MINIX_TEST)/kernel > 7run_kernel
	./mmvm -d $(MINIX_TEST)/kernel > my_kernel
	diff 7run_kernel my_kernel

## RUN TEST

t5test:
	7run -m target5 >7run_target5 2>&1
	./mmvm -m target5 >my_target5 2>&1
	diff 7run_target5 my_target5
t6test:
	7run -m target6 >7run_target6 2>&1
	./mmvm -m target6 >my_target6 2>&1
	diff 7run_target6 my_target6
t7test:
	7run -m target7 >7run_target7 2>&1
	./mmvm -m target7 >my_target7 2>&1
	diff 7run_target7 my_target7

c1test: testc1
	7run -m c1 >7run_c1 2>&1
	./mmvm -m c1 >my_c1 2>&1 
	diff 7run_c1 my_c1

c2test: testc2
	7run -m c2 >7run_c2 2>&1
	./mmvm -m c2 >my_c2 2>&1 
	diff 7run_c2 my_c2

c3test: testc3
	7run -m c3 >7run_c3 2>&1
	./mmvm -m c3 >my_c3 2>&1 
	diff 7run_c3 my_c3

c4test: testc4
	7run -m c4 >7run_c4 2>&1
	./mmvm -m c4 >my_c4 2>&1 
	diff 7run_c4 my_c4

c5test: testc5
	7run -m c5 >7run_c5 2>&1
	./mmvm -m c5 >my_c5 2>&1 
	diff 7run_c5 my_c5

# MINIX
mc1: $(CTEST)/1.c
	$(M2CC) $^
	mv a.out mc1

mc2: $(CTEST)/2.c
	$(M2CC) $^
	mv a.out mc2

mc3: $(CTEST)/3.c
	$(M2CC) $^
	mv a.out mc3

mc4: $(CTEST)/4.c
	$(M2CC) $^
	mv a.out mc4

mc5: $(CTEST)/5.c
	$(M2CC) $^
	mv a.out mc5

mc6: $(CTEST)/6.c
	$(M2CC) $^
	mv a.out mc6

module: $(CTEST)/module.c
	$(M2CC) -c $^


mc1test: mc1
	7run -m mc1 >7run_mc1 2>&1
	./mmvm -m mc1 >my_mc1 2>&1 
	diff 7run_mc1 my_mc1

mc2test: mc2
	7run -m mc2 >7run_mc2 2>&1
	./mmvm -m mc2 >my_mc2 2>&1 
	diff 7run_mc2 my_mc2

mc3test: mc3
	7run -m mc3 >7run_mc3 2>&1
	./mmvm -m mc3 >my_mc3 2>&1 
	diff 7run_mc3 my_mc3

mc4test: mc4
	7run -m mc4 >7run_mc4 2>&1
	./mmvm -m mc4 >my_mc4 2>&1 
	diff 7run_mc4 my_mc4

mc5test: mc5
	7run -m mc5 >7run_mc5 2>&1
	./mmvm -m mc5 >my_mc5 2>&1 
	diff 7run_mc5 my_mc5

# for NM
nmtest: mc1
	7run -m nm mc1 >7run_nm_mc1 2>&1
	./mmvm -m nm mc1 >my_nm_mc1 2>&1 
	diff 7run_nm_mc1 my_nm_mc1

# for AR
artest: module
	./mmvm -r /usr/local/minix2 /usr/local/minix2/usr/bin/ar cr module.a module.o
	md5 module.a

# for Fork and Exec
fork: $(FORKTEST)/hello.c $(FORKTEST)/minixfork.c
	$(M2CC) -o hello $(FORKTEST)/hello.c
	$(M2CC) -o minixfork $(FORKTEST)/minixfork.c

# for CC
7runcc:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/bin/cc tests/c/1.c 2>&1

ncpptest:
	./7run -r /usr/local/minix2 /usr/local/minix2/usr/lib/ncpp -D_EM_WSIZE=2 -D_EM_PSIZE=2 -D_EM_SSIZE=2 -D_EM_LSIZE=4 -D_EM_FSIZE=4 -D_EM_DSIZE=8 -D__ACK__ -D__minix -D__i86 tests/c/1.c >cc1.i

irreltest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/irrel cc1.i 

necmtest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/ncem -L cc1.i cc1.k

nopttest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/nopt <  cc1.k > cc1.m

ncgtest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/ncg cc1.m cc1.s

astest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/as - cc1.s -o cc1.o

ldtest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/ld -i -o cc111.out /usr/lib/ncrtso.o cc1.o /usr/lib/libd.a /usr/lib/libc.a /usr/lib/libfp.a /usr/lib/libe.a /usr/lib/end.a

cvtest:
	./7run -m -r /usr/local/minix2 /usr/local/minix2/usr/lib/cv cc111.out a.out
	
.c.o:
	$(CXX) -c $*.c

clean:
	rm -f $(TARGET) *.o *~  
	rm -rf /tmp/ar*
#	rm -rf module.a
cleantarget:
	rm -rf target* 
