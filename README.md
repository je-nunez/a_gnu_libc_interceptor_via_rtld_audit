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

     1428930338.096336: 9768: Calling symbol malloc from address 7f31464617e3 ... (RDI=7, RSI=1, RDX=0, RCX=241128144088, ...)
          caller backtrace:
          [in order to call: malloc(RDI=7, RSI=1, RDX=0, RCX=241128144088, R8=139849601925344, R9=3)
          0: 0x7f31466827ab: la_x86_64_gnu_pltenter+0xed (./liba_glibc_rtld_audit_shared_lib.so.1.0.1)
          1: 0x3823a0f83e: _dl_profile_fixup+0x3ce (/lib64/ld-linux-x86-64.so.2)
          2: 0x3823a16c88: _dl_runtime_profile+0x8e8 (/lib64/ld-linux-x86-64.so.2)
          3: 0x7f31464617e3: funct_in_shared_libr_declared_compiling_time+0x23 (./libshared_library_declared_at_compiling_time.so)
          4: 0x3823a16d46: _dl_runtime_profile+0x9a6 (/lib64/ld-linux-x86-64.so.2)
     1428930338.096831: 9768: Returning from symbol malloc ...
          profiling call of malloc:
             user-mode time spent: 0.000000
             kernel-mode time spent: 0.000000


where RDI, RSI, are the values of the CPU registers in the call to `malloc` above
(e.g. in the above call to `malloc()`, the requested amount of memory is set in
the register `RDI`, with value `RDI=10240` in the above line, so the call was
`malloc(10240)`. See
http://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI
on the passing of the actual arguments in a function call preferently through
the CPU registers directly, avoiding if possible the stack, in Linux -and other
Unix-like O.S.)

# Pre-requisites

The `stack-backtrace` of the caller-procedures uses the `libunwind` shared library,
so this one needs to be installed in order to run. This is commonly available via
your package manager, eg., for RedHat and Debian:

        RedHat:

             yum install libunwind libunwind-devel

        Debian:

             apt-get install libunwind8 libunwind8-dev

# Tracking Pending things

Trying to catch a logical bug in `la_x86_64_gnu_pltexit(...)` that is preventing
its execution. `la_symbind64(...)` seems to prepare for `la_x86_64_gnu_pltexit(...)`
to prepare for `la_x86_64_gnu_pltexit(...)` to be called (as `la_x86_64_gnu_pltenter(...)`
is indeed invoked before each call, and both `*_gnu_pltenter()` and `*_gnu_pltexit()`
are expected to be quite orthogonal if `la_symbind64(...)` requests that they both be
invoked).

Read also the code around glibc's elf/dl-runtime.c

    https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/dl-runtime.c
								
to see what is happening with the above invocation, and opened a bug:

      https://sourceware.org/bugzilla/show_bug.cgi?id=18177

(other people has appended more details on the LD_AUDIT implementation in
dl-runtime.c) but this bug seems to be unrelated to my logical bug
described in the first paragraph that I'm still investigating here.

In the `stack-backtrace` of the caller-procedures, the unwind of the caller stack
is being done and shown on the caller stack of the Linux dynamic loader. This
stack-backtrace has to be fixed on the caller stack in `glibc` itself up to `main()`.
Ie., in the test program `a_test.c` in this repository, the stack-backtrace
shows the procedures `la_x86_64_gnu_pltenter` (itself) and `_dl_profile_fixup`
and `_dl_runtime_profile` belonging to the dynamic linker `/lib64/ld-linux-x86-64.so`,
whereas it should really omit these inner details in the stack-backtrace and print
instead the intention of entering the procedure (`__symname`) (ie., replacing the
inner lines `0:`, `1:`, and `2:` below with just the intention of calling proc
`__symname`):

     caller backtrace:
       0: 0x92b06229: la_x86_64_gnu_pltenter+0xde (./liba_glibc_rtld_audit_shared_lib.so.1.0.1)
       1: 0x23a0f83e: _dl_profile_fixup+0x3ce (/lib64/ld-linux-x86-64.so.2)
       2: 0x23a16c88: _dl_runtime_profile+0x8e8 (/lib64/ld-linux-x86-64.so.2)
       3: 0x40067f: test_function_inner+0x69 (./a_test)
       4: 0x4006b7: test_function_outer+0x20 (./a_test)
       5: 0x400740: main+0x87 (./a_test)

(It is using `libunwind`, not the default glibc `backtrace` because the latter
is not Posix safe in the context where this tracer needs to use -namely, `backtrace`
is `AS-Unsafe init heap dlopen plugin lock` in the context where this tracer
needs to use it:

     libunwind:

         http://www.nongnu.org/libunwind/man/libunwind%283%29.html

     glibc bactrace safety by context:

         http://www.gnu.org/software/libc/manual/html_node/Backtraces.html
         http://www.gnu.org/software/libc/manual/html_node/POSIX-Safety-Concepts.html

Another option for the stack-trace would be to use the `DynInst`'s `StackWalkerAPI`:

     http://www.dyninst.org/sites/default/files/manuals/dyninst/StackwalkerAPI.pdf

This stack-trace is in-core, for this shared-library runs under the same thread
and in the same local memory space of the thread/process it is tracing, not an
external thread/process, so `libunwind-ptrace` (used by `strace`, `gdb`, and
`ltrace`) seems to be too heavy-weight for this `same thread, same local memory
space` case, for it uses the `ptrace` system-call and allows to backtrace
externals `thread-ids` -and it stops the external thread, etc:

     libunwind-ptrace:

         http://www.nongnu.org/libunwind/man/libunwind-ptrace%283%29.html

     underlying ptrace system-call:

         http://man7.org/linux/man-pages/man2/ptrace.2.html

We need to clean-up a little the report of this auditing library in the standard-output,
it is too much verbose:


     1428903995.183209: <thread>: Calling symbol <function_name_in_shared_libr> ...
        caller backtrace:
          <caller-stack-backtrace-up-to-this-function>
          ...
     1428903995.183627: <thread>: Returning from symbol <function_name_in_shared_libr>
        profiling call of <function_name_in_shared_libr>:
           user-mode time spent: 0.000008
           kernel-mode time spent: 0.000386

(For an alternative method of profiling, but on only one shared library, see the
environment variable `LD_PROFILE` to the glibc dynamic loader library, `ld.so`:

      http://man7.org/linux/man-pages/man8/ld.so.8.html

)

# Overhead added by this auditor

Using `perf record -g ...` to see the overhead added:

      8.11%  a_test   libunwind.so.8.0.1                 [.] _Uelf64_get_proc_name_in_image
      4.18%  a_test   [kernel.kallsyms]                  [k] format_decode
      4.09%  a_test   [kernel.kallsyms]                  [k] number.isra.2
      3.83%  a_test   [kernel.kallsyms]                  [k] memcpy
      3.77%  a_test   [kernel.kallsyms]                  [k] vsnprintf
      2.72%  a_test   libunwind.so.8.0.1                 [.] _Ux86_64_get_elf_image

So the old load using `snprintf` (which appeared in `perf record` as much higher
loads from `format_decode()` and `vsnprintf()` than appear now) diminished when
we replaced the use of `snprintf` in the profiling sections with a custom
procedure for our case (there are other sections of the code that still use
`snprintf`). The `perf record` also shows the `libunwind`'s 
`_Uelf64_get_proc_name_in_image()` used in the caller-stack trace: probably some
caching in order to avoid calling this procedure in `libunwind` when the 
`Instr-Pointer` address of the call is already contained in some range in
the cache may help in preventing the call to `_Uelf64_get_proc_name_in_image()`,
although for this caching we need to intercept also other functions, like
`dlclose()` and `dlopen()`, which make the cache of ranges (mappings) of
`Instr-Pointers` per procedure name to be invalidated.

Eg., the caching of ranges of IPs per procedure names (in order to decrease
the overhead by this auditor in the caller-stack trace and
`_Uelf64_get_proc_name_in_image()`) can be done using:

      int unw_get_proc_info(unw_cursor_t *cp, unw_proc_info_t *pip);

      ( http://www.nongnu.org/libunwind/man/unw_get_proc_info%283%29.html )

which returns in the `unw_proc_info_t *pip` parameter, the `unw_word_t start_ip`
and `unw_word_t end_ip` -ie., the range of Instr-Pointers- of the procedure
being transversed in the caller-stack trace. This function `unw_get_proc_info()`
in `libunwind` is called just before `unw_get_proc_name()` in the code in this
project:

      #include <libunwind.h>
      ...

      show_caller_stack_backtrace(...)
      {
            ...
            while (unw_step(&stack_cursor) > 0)
            {
                 unw_get_reg(&stack_cursor, UNW_REG_IP, &instr_ptr);
                 ...
                 ret = unw_get_proc_info(&stack_cursor, &procedure_info);
                 ...
                 ret = unw_get_proc_name(&stack_cursor, procedure_name,
                                         sizeof procedure_name,
                                         &proced_offset_of_call);
                 ...
            }
            ...
      }

so our code, in the first `unw_get_reg()` finds the `&instr_ptr` of the call,
then in `unw_get_proc_info()` finds the `procedure_info` of the range of IPs
where this procedure starts and ends (man page above on `unw_get_proc_info()`)
and in the last call `unw_get_proc_name()`, finds the `procedure_name` associated
with this range of code, but currently it does not cache these results to prevent
future calls again to `unw_get_proc_info()` and `unw_get_proc_name()` for the
same range of code, and this is what `perf record` has been telling us that is
expensive. (As said above, we need to intercept functions like `dlclose` and
`dlopen` to invalidate part of the cache.)

