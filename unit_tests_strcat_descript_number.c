
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The header files for the module we want to test */

#include "module_strcat_descript_number.h"

int
main(void)
{
    int number_of_tests = 0, failed_tests = 0;

    char buffer[1024];
    int n;


    number_of_tests++;
    memset(buffer, 0, sizeof buffer);
    n = strcat_descript_number(buffer, "a", 1);

    if (strcmp(buffer, " a: 1") != 0) {
       printf("Expected:\n    %s (length %d)==\nComputed:\n    %s== (length %d)\n", " a: 1", strlen(" a: 1"), buffer, n);
       failed_tests++;
    }


    number_of_tests++;
    memset(buffer, 0, sizeof buffer);
    n = strcat_descript_number(buffer, "test A", 456);

    if (strcmp(buffer, " test A: 456") != 0) {
       printf("Expected:\n    %s (length %d)==\nComputed:\n    %s== (length %d)\n", " test A: 456", strlen(" test A: 456"), buffer, n);
       failed_tests++;
    }


    number_of_tests++;
    memset(buffer, 0, sizeof buffer);
    n = strcat_descript_number(buffer, "my-description-Z", -123456789);

    if (strcmp(buffer, " my-description-Z: -123456789") != 0) {
       printf("Expected:\n    %s (length %d)==\nComputed:\n    %s== (length %d)\n", " my-description-Z: -123456789", strlen(" my-description-Z: -123456789"), buffer, n);
       failed_tests++;
    }


    number_of_tests++;
    memset(buffer, 0, sizeof buffer);
    n = strcat_descript_number(buffer, "A-label", 0);

    if (strcmp(buffer, " A-label: 0") != 0) {
       printf("Expected:\n    %s (length %d)==\nComputed:\n    %s== (length %d)\n", " A-label: 0", strlen(" A-label: 0"), buffer, n);
       failed_tests++;
    }


    /* All unit-tests of strcat_descript_number() are done */
    printf("\nAll unit-tests of strcat_descript_number() are done: "
	   "total %d, failed %d tests\n", number_of_tests, failed_tests);

    return (failed_tests==0)? 0: 1;
}

