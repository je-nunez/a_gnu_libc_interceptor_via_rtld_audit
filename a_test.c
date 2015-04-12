
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "shared_library_declared_at_compiling_time.h"

int
test_function_inner(char **arr, int size)
{
    /* FIRST: 
     * call a function in a shared library but whose information is 
     * linked at compiling-time */
    int i;
    for (i=0; i<size; i++) {

        enum function_result res ;
        res = funct_in_shared_libr_declared_compiling_time(i);

        if (res == ERROR) return -1 ;
    }
    

    /* SECOND: 
     * call a function in a shared library loaded dynamically */
    void *sh_libr_handle;
    int (*dyn_funct_ptr)(unsigned int);
    char *error;

    sh_libr_handle = dlopen("./libshared_library_dynamic_loading.so", RTLD_LAZY);
    if (!sh_libr_handle) 
    {
       fprintf(stderr, "%s\n", dlerror());
       return -2 ;
    }

    dlerror();
    dyn_funct_ptr = dlsym(sh_libr_handle, "funct_in_shared_libr_dynamically_loaded");
    if ((error = dlerror()) != NULL)  
    {
       fprintf(stderr, "%s\n", error);
       return -3 ;
    }

    int fn_result ; 
    fn_result = (*dyn_funct_ptr)(20);
    printf("Result of dynamic function =%d\n", fn_result);

    dlclose(sh_libr_handle);

    return 0;
}


int
test_function_outer(char **arr, int size)
{
     return test_function_inner(arr, size);
}


int
main(int argc, char *argv[])
{
    pid_t curr_pid = getpid();
    printf("Entering main with pid=%d\n", curr_pid);

    char * test[10];
    int i;
    for (i=0; i<10; i++) {

        test[i] = (char *) malloc( (i+1) * 10 );

        free( test[i] );
    }

    test_function_outer(test, 10);

    return 0;
}

