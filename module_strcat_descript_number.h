
#ifndef MODULE_STRCAT_DESCRIPT_NUMBER_H

#define MODULE_STRCAT_DESCRIPT_NUMBER_H

extern int
strcat_descript_number(char * destin, char * description, long number);

    /* This strcat_descript_number() is an optimization which implements:
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

#endif

