
CC = gcc
CFLAGS = -g -Wall


.SILENT:  help


help:
	echo "Makefile help"	
	echo -e "Possible make targets:\n"	
	echo "    make liba_glibc_rtld_audit_shared_lib.so.1.0.1"	
	echo -e "         Build the shared library to the current directory.\n"	
	echo "    make a_test"	
	echo -e "         Build the test program to the current directory.\n"	
	echo "    make run_a_test"	
	echo -e "         Build the both above -if necessary- and run a glibc audit on the test program.\n"	
	echo "    make clean"	
	echo -e "         Remove compiled and binary-object files.\n"	
	echo "    make help"	
	echo -e "         Offers these help instructions.\n"	


liba_glibc_rtld_audit_shared_lib.so.1.0.1: a_glibc_rtld_audit_shared_lib.c
	$(CC) -fPIC -c  $(CFLAGS) -o  a_glibc_rtld_audit_shared_lib.o  a_glibc_rtld_audit_shared_lib.c
	$(CC) -shared -Wl,-soname,liba_glibc_rtld_audit_shared_lib.so.1 -o liba_glibc_rtld_audit_shared_lib.so.1.0.1 a_glibc_rtld_audit_shared_lib.o -lc


a_test: a_test.c
	$(CC)  $(CFLAGS)  -o a_test  a_test.c


.PHONY : clean


clean:
	-rm -f a_test  a_test.o  a_glibc_rtld_audit_shared_lib.o   liba_glibc_rtld_audit_shared_lib.so.1.0.1


run_a_test: a_test   liba_glibc_rtld_audit_shared_lib.so.1.0.1
	LD_LIBRARY_PATH=`pwd` LD_AUDIT=liba_glibc_rtld_audit_shared_lib.so.1.0.1 ./a_test
     
