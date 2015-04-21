
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* TODO:
 * Probably a better modularization for including 
 *        strcat_descript_number()
 * from the main program here in the unit-tests.
 * E.g., TODO a
 * 
 *      #include "strcat_descript_number.c"
 *
 */
static int
strcat_descript_number(char * destin, char * description, long number)
{
    /* Implements:
     *     return snprintf(destin, remaining_space,
     *                     " " description ": %ld", number);
     */

    int n=1;
    /* Append a space */
    *destin++ = ' ';
    /* Append the description */
    while (*description != '\0') {
        *destin++ = *description++;
        n++;
    }
    /* Append a colon and a space */
    *destin++ = ':';
    *destin++ = ' ';
    n+=2;
    /* Convert "number" */
    if (number == 0) { *destin++='0'; *destin++='\0'; return ++n; }
    else if (number < 0) { *destin++='-'; number= -number; n++; }

    /* Convert "number" to a temporary string */

    char n_to_s[51];     /* 51 is large enough for an int128 */
    int i = -1, rem;
    while (number != 0)
    {
        rem = number % 10;
        number /= 10;
        n_to_s[++i] = rem + '0';
    }

    n+=i;
    for (; i >= 0; i--) *destin++ = n_to_s[i];
    *destin = '\0';

    return n;
}


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

