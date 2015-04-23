
extern int
strcat_descript_number(char * destin, char * description, long number)
{
    /* This is an optimization which implements:
     * 
     *     return sprintf(destin, " %s: %ld", description, number);
     *  
     * since it is called repeatedly. It doesn't need to implement the
     * version of:
     * 
     *          snprintf(destin, max_size,...) 
     * because its callee,
     *          print_profiling_cost_between_two_snapshots_in_time()
     * passes to it its local string buffer "output_buff[2048]", 
     * which has enough space for the formatted result. See comment
     * on this variable "output_buff[2048]" below.
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

