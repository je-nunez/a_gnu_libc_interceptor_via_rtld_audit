
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "shared_library_declared_at_compiling_time.h"

enum function_result
funct_in_shared_libr_declared_compiling_time(unsigned int size)
{
    char * ptr ;
    if (size < 1) return NOTHING_TO_DO; /* nothing to do */

    ptr = (char *)malloc(size);
    if (ptr) {
          free(ptr);
          return OK;    /* operation was sucessful */
    } else {
          perror("ERROR: funct_shared_libr_declared_at_compiling_time");
          return ERROR;    /* 0 = error condition */
    }
}

