# A GNU libc interceptor via RTLD-Audit 

# WIP

This project is a *work in progress*. The implementation is *incomplete* and subject to change. The documentation can be inaccurate.

# Description

A GNU GLibc Audit shared-library for Linux using the RTLD-Audit mechanism in GLibc.

To see `make` targets, run

      make help

Much of the information about the GNU GLibc Audit is inside the source code for 
its dynamic loader at dlopen() time, `rtld.c`:

     https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/rtld.c#l1335
         
        ...
         // Follow the usage of audit_iface_names[] inside rtld.c for Audit API
         
         static const char audit_iface_names[] =
                     "la_activity\0"
                     "la_objsearch\0"
                     "la_objopen\0"
                     ...

which prepares for the use of the Glibc Audit inside the runtime-linker; and
inside the code of the dynamic-symbol actual usage (auditing) during execution,
`dl-runtime.c`:

     https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/dl-runtime.c#l383
         
       ...
       /* Follow the usage of the ARCH_LA_PLT* macros, which in turn are defined
        * in the header file:
        * 
        *    <glibc-source-code>/sysdeps/<mach-architecture>/dl-machine.h
        * 
        * for each architecture where GLibc has been ported to.
        * 
        * E.g., this code in ./elf/dl-runtime.c audits the "entrance" (calling) 
        * into the symbol "sym" in glibc:
        */
        ...  afct->ARCH_LA_PLTENTER (&sym, reloc_result->boundndx,
                                           ...
        

which defines the actual use of the Glibc Audit inside the runtime-linker. Also,
the interfaces for this mechanism,

     /usr/include/link.h
     /usr/include/bits/link.h

and the documentation in the `rtld-audit(7)` man page. E.g., below the comment 
in `/usr/include/link.h`:

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

or alternatively:

     make  run_a_test

and you will see some lines like:

     1427243201.652328: Calling symbol malloc from address 4006a0 ... (RDI=10240, RSI=0, RDX=11725120, RCX=11725120...

where RDI, RSI, are the values of the CPU registers in the call to `malloc` above 
(e.g. in the above call to `malloc()`, the requested amount of memory is set in 
the register `RDI`, with value `RDI=10240` in the above line, so the call was 
`malloc(10240)`. See 
http://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI 
on the passing of the actual arguments in a function call preferently through 
the CPU registers directly, avoiding if possible the stack, in Linux -and other 
Unix-like O.S.)

