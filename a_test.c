
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>



int
test_function_inner(char **arr, int size)
{
    int i;
    for (i=0; i<size; i++) {

        arr[i] = (char *) malloc((i+20) * 10);

        free(arr[i]);
    }
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

