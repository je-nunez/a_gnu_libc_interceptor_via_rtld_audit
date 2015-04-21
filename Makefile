
CC = gcc
CFLAGS = -g -Wall -I.
LDFLAGS = -L/usr/lib64/ -L .


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
	echo "    make run_a_test_low_level"	
	echo -e "         Similar to 'run_a_test' but also debug how the std dynamic-linker is binding symbols\n"	
	echo "    make unit_tests"
	echo -e "         Perform unit-tests of different sections of the main code in 'a_glibc_rtld_audit_shared_lib.c'"
	echo "    make clean"	
	echo -e "         Remove compiled and binary-object files.\n"	
	echo "    make help"	
	echo -e "         Offers these help instructions.\n"	


liba_glibc_rtld_audit_shared_lib.so.1.0.1: a_glibc_rtld_audit_shared_lib.c
	$(CC) -fPIC -c  $(CFLAGS) -o  a_glibc_rtld_audit_shared_lib.o  a_glibc_rtld_audit_shared_lib.c
	$(CC) -shared   $(LDFLAGS) -Wl,-soname,liba_glibc_rtld_audit_shared_lib.so.1 -o liba_glibc_rtld_audit_shared_lib.so.1.0.1 a_glibc_rtld_audit_shared_lib.o -lc  -ldl  -lunwind 


a_test: a_test.c  shared_library_declared_at_compiling_time.c  shared_library_declared_at_compiling_time.h   shared_library_dynamic_loading.c   shared_library_dynamic_loading.h
	$(CC) -fPIC -c $(CFLAGS)  shared_library_declared_at_compiling_time.c
	$(CC) -shared  $(LDFLAGS) -Wl,-soname,libshared_library_declared_at_compiling_time.so -o libshared_library_declared_at_compiling_time.so shared_library_declared_at_compiling_time.o -lc

	$(CC) -fPIC -c $(CFLAGS)  shared_library_dynamic_loading.c
	$(CC) -shared  $(LDFLAGS) -Wl,-soname,libshared_library_dynamic_loading.so -o libshared_library_dynamic_loading.so shared_library_dynamic_loading.o -lc

	$(CC) -c  $(CFLAGS)  -o a_test.o a_test.c  
	$(CC) -rdynamic $(LDFLAGS)  -o a_test a_test.o  -lshared_library_declared_at_compiling_time   -ldl


.PHONY : clean


clean:
	-rm -f a_test  a_test.o  a_glibc_rtld_audit_shared_lib.o   liba_glibc_rtld_audit_shared_lib.so.1.0.1 shared_library_declared_at_compiling_time.o  libshared_library_declared_at_compiling_time.so   shared_library_dynamic_loading.o   libshared_library_dynamic_loading.so  unit_tests_strcat_descript_number

run_a_test: a_test   liba_glibc_rtld_audit_shared_lib.so.1.0.1
	LD_LIBRARY_PATH=`pwd` LD_AUDIT=liba_glibc_rtld_audit_shared_lib.so.1.0.1 ./a_test


run_a_test_low_level: a_test   liba_glibc_rtld_audit_shared_lib.so.1.0.1
	LD_LIBRARY_PATH=`pwd` LD_DEBUG=all LD_AUDIT=liba_glibc_rtld_audit_shared_lib.so.1.0.1 ./a_test

unit_tests: unit_tests_strcat_descript_number.c
	-echo "These are unit-tests of different sections of the main code in 'a_glibc_rtld_audit_shared_lib.c'"
	-echo 'Doing unit-test of strcat_descript_number()'
	gcc -o unit_tests_strcat_descript_number   unit_tests_strcat_descript_number.c
	./unit_tests_strcat_descript_number

