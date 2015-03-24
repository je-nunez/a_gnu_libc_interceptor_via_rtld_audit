
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    printf("In main\n");
    
    char * test[10];
    int i;
    for (i=0; i<10; i++)
        test[i] = (char *) malloc( (i+1) * 10 );

    test[0] = (char *) malloc( 10 );
    test[1] = (char *) malloc( 40 );
    test[2] = (char *) malloc( 80 );
    test[3] = (char *) malloc( 160 );
    test[4] = (char *) malloc( 320 );
    test[5] = (char *) malloc( 640 );
    test[6] = (char *) malloc( 1280 );
    test[7] = (char *) malloc( 2560 );
    test[8] = (char *) malloc( 5120 );
    test[9] = (char *) malloc( 10240 );

    free( test[0] );
    free( test[1] );
    free( test[2] );
    free( test[3] );
    free( test[4] );
    free( test[5] );
    free( test[6] );
    free( test[7] );
    free( test[8] );
    free( test[9] );
}

