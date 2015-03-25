# A GNU libc interceptor via RTLD-Audit 

A GNU GLibc Audit shared-library for Linux using the RTLD-Audit mechanism in GLibc.

Much of this information is in:

     /usr/include/link.h
     /usr/include/bits/link.h

and also in the `rtld-audit(7)` man page. E.g., below the comment in 
`/usr/include/link.h`:

     /* Prototypes for the ld.so auditing interfaces.  These are not
       defined anywhere in ld.so but instead have to be provided by the
       auditing DSO.  */
     ...

Note: the header `/usr/include/bits/link.h` shouldn't be explicity included
in your program, `/usr/include/link.h` includes it and 
`/usr/include/bits/link.h` errors out during compilation if 
`/usr/include/link.h` is also included:

     /usr/include/bits/link.h
       ...
       #ifndef _LINK_H
       # error "Never include <bits/link.h> directly; use <link.h> instead."
       #endif

The file `a_test.c` contains only a test (with a lot of memory-leaking) just to
see how the audit library reports it. You could use:

     export LD_LIBRARY_PATH=`pwd`
     LD_AUDIT=liba_glibc_rtld_audit_shared_lib.so.1.0.1 ./a_test

and you will see some lines like:

     1427243201.652328: Calling symbol malloc from address 4006a0 ... (RDI=10240, RSI=0, RDX=11725120, RCX=11725120...

where RDI, RSI, are the values of the CPU registers in the call to `malloc` above 
(e.g. in the above call to `malloc()`, the requested amount of memory is set in the
register `RDI`, with value `RDI=10240` in the above line, so the call was 
`malloc(10240)`. See 
http://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI 
for the passing of parameters directly through the CPU registers in GCC in Linux.)

