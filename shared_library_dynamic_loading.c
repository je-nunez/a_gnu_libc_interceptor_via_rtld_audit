
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "shared_library_dynamic_loading.h"


int
funct_in_shared_libr_dynamically_loaded(unsigned int numb_elements)
{
    char * ptr ;
    if (numb_elements < 1) return 0; /* nothing to do */

    ptr = (char *)calloc(sizeof(char *), numb_elements);
    if (ptr) {
          free(ptr);
          return 1;    /* operation was sucessful */
    } else {
          perror("ERROR: funct_in_shared_libr_dynamically_loaded");
          return -1;    /* -1 = error condition */
    }
}

