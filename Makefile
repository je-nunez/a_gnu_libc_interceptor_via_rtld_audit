
CC = gcc
CFLAGS = -g -Wall

liba_glibc_rtld_audit_shared_lib.so.1.0.1: a_glibc_rtld_audit_shared_lib.c
	$(CC) -fPIC -c  $(CFLAGS) -o  a_glibc_rtld_audit_shared_lib.o  a_glibc_rtld_audit_shared_lib.c
	$(CC) -shared -Wl,-soname,liba_glibc_rtld_audit_shared_lib.so.1 -o liba_glibc_rtld_audit_shared_lib.so.1.0.1 a_glibc_rtld_audit_shared_lib.o -lc


a_test: a_test.c
	$(CC)  $(CFLAGS)  -o a_test  a_test.c


.PHONY : clean

clean:
	rm -f a_test  a_test.o  a_glibc_rtld_audit_shared_lib.o   liba_glibc_rtld_audit_shared_lib.so.1.0.1


run_a_test: a_test   liba_glibc_rtld_audit_shared_lib.so.1.0.1
	export LD_LIBRARY_PATH=`pwd`
	LD_AUDIT=liba_glibc_rtld_audit_shared_lib.so.1.0.1 ./a_test
     
