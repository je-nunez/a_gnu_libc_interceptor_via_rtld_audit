
#ifndef  SHARED_LIBRARY_LINKED_AT_COMPILING_TIME_H__
#define  SHARED_LIBRARY_LINKED_AT_COMPILING_TIME_H__

enum function_result {
   ERROR = 0,
   OK = 1,
   NOTHING_TO_DO = 2
};

extern enum function_result
funct_in_shared_libr_declared_compiling_time(unsigned int size);

#endif

