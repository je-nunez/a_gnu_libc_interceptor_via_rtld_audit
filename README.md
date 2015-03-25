# A GNU libc interceptor via RTLD-Audit 

A GNU GLibc Audit shared-library for Linux using the RTLD-Audit mechanism in GLibc.

Much of this information is in:

   /usr/include/link.h
   /usr/include/bits/link.h

and also in the `rtld-audit(7)` man page. E.g., in the comment in 
`/usr/include/link.h`:

   /* Prototypes for the ld.so auditing interfaces.  These are not
      defined anywhere in ld.so but instead have to be provided by the
      auditing DSO.  */

Note: the header `/usr/include/bits/link.h` shouldn't be explicity included
in your program, `/usr/include/link.h` includes it and 
`/usr/include/bits/link.h` errors out during compilation if 
`/usr/include/link.h` is also included:

   /usr/include/bits/link.h
      ...
      #ifndef _LINK_H
      # error "Never include <bits/link.h> directly; use <link.h> instead."
      #endif


