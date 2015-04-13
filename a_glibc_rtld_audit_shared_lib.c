
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <link.h>
#include <elf.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>

#ifndef __USE_BSD
#define __internal_BSD_was_not_in_use
#define __USE_BSD
#endif 
    
#include <sys/time.h>
    /* We need timersub() from sys/time.h, available only when __USE_BSD */
 
#ifdef __internal_BSD_was_not_in_use
#undef __USE_BSD
#endif 



/* UNW_LOCAL_ONLY: Unwind caller stack-trace of local architecture only, not 
 * cross-platform */

#define UNW_LOCAL_ONLY

#include <libunwind.h>
#include <dlfcn.h>

#define MAX_PROCEDURE_NAME_LENGTH  4096
 

/* This is to track the profiling of time spent inside the procedures in the stack */

struct track_calling_stack {
       char * procedure_name;   /* this is to verify the name of proced. in stack */
       struct rusage profiling_data_at_entry_in_call;  /* profiling data itself */
       struct track_calling_stack * next_in_stack; /* TODO: avoid heap fragment */
};

/* static __thread unsigned levels_in_thread_caller_stack = 0;  // for avoiding heap fragmentation */
static __thread struct track_calling_stack * thread_caller_stack = NULL;

/* TODO 222:
 *   Note: in the above "struct track_calling_stack" there could be a very subtle
 *         semantic mistake in the verification field "procedure_name" in the stack,
 *         because the __another__ glibc audit-library, in its pltenter(), can
 *         change the name of the "function_name" called, ---
 *
 *                 From man page of rtld-audit(7):
 *
 *                     The return value of la_pltenter() is as for la_symbind*(). 
 *                     --comment: ie., to change the name to which to record 
 *                                this "function_name"
 *
 *         so the name saved by this glibc audit-library in the field 
 *         "procedure_name" can be different to the name "function_name" that a
 *         __another__ glibc audit-library altered, so the pltexit() in this
 *         library will end up seeing the modified "function_name" and not
 *         the "procedure_name" it had expected, as recorded by pltenter() here.
 *         MAY this case described above happen? TODO: run a test with a second
 *         dummy audit lib which only does is to redirect the "function_name".
 */

static pid_t
__gettid(void)
{
  /* returns the kthread id or pid() if no kthread */
  /* uses the system-call to get the underlying 
   * kthread() to this user-land execution thread */
  pid_t tid;
  tid = syscall(SYS_gettid);
  return tid;
}


extern unsigned int
la_version(unsigned int version)
{
  return (LAV_CURRENT);
}
 
extern unsigned int 
la_objopen(struct link_map *__map, Lmid_t __lmid, uintptr_t *__cookie)
{

  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: la_objopen(): loading \"%s\"; lmid = %s; cookie=%lx\n",
            tp.tv_sec, tp.tv_usec, kernel_thread_id,
            (__map->l_name[0] != '\0') ? __map->l_name : "MAIN-APPLIC",
            (__lmid == LM_ID_BASE) ?  "Link map is part of the initial namespace" :
            (__lmid == LM_ID_NEWLM) ? "Link map is part of a new namespace requested via dlmopen" :
            "???",
            (unsigned long int) __cookie);

  /* request to audit symbols in this object, and external symbols referenced 
   * from this object. */
  return (LA_FLG_BINDTO | LA_FLG_BINDFROM);
}

extern unsigned int 
la_objclose(uintptr_t *cookie)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: Closing object with cookie %lx\n", 
         tp.tv_sec, tp.tv_usec, kernel_thread_id, (unsigned long int) cookie);
  return (0);
}

extern void 
la_preinit(uintptr_t *cookie)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: Before calling main() with cookie %lx\n", 
         tp.tv_sec, tp.tv_usec, kernel_thread_id, (unsigned long int) cookie);
}


extern char *
la_objsearch(const char *__name, uintptr_t *__cookie, unsigned int __flag)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: Searching for %s with cookie %lx\n", 
         tp.tv_sec, tp.tv_usec, kernel_thread_id, __name, 
         (unsigned long int) __cookie);

  /* From man page of rtld-audit(7):
   *
   * As its function result, la_objsearch() returns the pathname that the 
   * dynamic linker should use for further processing. If NULL is returned, 
   * then this pathname is ignored for further processing. If this audit 
   * library simply intends to monitor search paths, then name should be 
   * returned. 
   */

  return (char *)__name;
}


extern uintptr_t 
la_symbind64(Elf64_Sym *__sym, unsigned int __ndx,
                               uintptr_t *__refcook, uintptr_t *__defcook,
                               unsigned int *__flags, const char *__symname)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: la_symbind64(): symname = %s; sym->st_value = %p",
         tp.tv_sec, tp.tv_usec, kernel_thread_id, __symname, 
         (void *)__sym->st_value);
  printf("        ndx = %d; flags = 0x%x", __ndx, *__flags);
  printf("; caller-cook = %lx; callee-cook = %lx\n", 
	 (unsigned long int) __refcook, (unsigned long int) __defcook);

  /*
   * For debug: don't trace calls to, or returns from, this symbol: 
        *__flags = (LA_SYMB_NOPLTENTER | LA_SYMB_NOPLTEXIT);
   *
   */
  *__flags = 0;

  /* From man page of rtld-audit(7):
   *
   * The return value of la_symbind32() and la_symbind64() is the address to 
   * which control should be passed after the function returns. If the auditing 
   * library is simply monitoring symbol bindings, then it should return 
   * sym->st_value. A different value may be returned if the library wishes to 
   * direct control to an alternate location. */
  return __sym->st_value;
}


static void
show_caller_stack_backtrace(const char *function_to_be_called, 
			    uintptr_t *func_library_cookie, 
			    La_x86_64_regs *calling_registers)
{
  unw_cursor_t  stack_cursor;
  unw_context_t current_context;
  unw_word_t    instr_ptr;
  /* unw_word_t    stack_ptr;   TODO: unused at the moment */

  char procedure_name[MAX_PROCEDURE_NAME_LENGTH];
  unw_word_t    proced_offset_of_call;
  unw_proc_info_t procedure_info;
  Dl_info symb_info;
  const char * filename;
  int ret;

  procedure_info.unwind_info = NULL;
  ret = unw_getcontext(&current_context);
  if (ret) {
      fprintf(stderr, "unw_getcontext failed: %s [%d]\n", unw_strerror(ret), ret);
      return;
  }

  ret = unw_init_local(&stack_cursor, &current_context);
  if (ret) {
      fprintf(stderr, "unw_init_local failed: %s [%d]\n", unw_strerror(ret), ret);
      return;
  }

  /* do the caller stack backtrace, and, since this procedure is internally
   * called through the procedures in the dynamic linker, explain also with
   * what intent these internal procedures are called (ie., in order to call
   * `function_to_be_called` */
  La_x86_64_regs *r = calling_registers;
  printf("     caller backtrace:\n"
    	 "       [in order to call: %s(RDI=%lu, RSI=%lu, RDX=%lu, RCX=%lu, R8=%lu, R9=%lu)\n",
	 function_to_be_called, r->lr_rdi, r->lr_rsi, r->lr_rdx, r->lr_rcx, 
	 r->lr_r8, r->lr_r9 );

  unsigned  caller_depth = 0;
  while (unw_step(&stack_cursor) > 0) 
  {
     unw_get_reg(&stack_cursor, UNW_REG_IP, &instr_ptr);
     /* TODO: 
      *     analysis of arguments at this stack frame &stack_cursor,
      *     although this may be difficult because they could have
      *     been passed through CPU registers and not through the stack.
      * 
         unw_get_reg(&stack_cursor, UNW_REG_IP, &stack_ptr); */

     ret = unw_get_proc_info(&stack_cursor, &procedure_info);
     if (ret) {
         printf("ERROR: unw_get_proc_info: %s [%d]\n", unw_strerror(ret), ret);
         break;
     }

     ret = unw_get_proc_name(&stack_cursor, procedure_name, 
			     sizeof procedure_name, &proced_offset_of_call);
     if (ret && ret != -UNW_ENOMEM) {
         if (ret != -UNW_EUNSPEC) printf("ERROR: unw_get_proc_name: %s [%d]\n", 
					 unw_strerror(ret), ret);
         procedure_name[0] = procedure_name[1] = procedure_name[2] = '?';
         procedure_name[3] = 0;
     }

     if (dladdr((void *)(procedure_info.start_ip + proced_offset_of_call), 
		&symb_info) && symb_info.dli_fname && *symb_info.dli_fname)
         filename = symb_info.dli_fname;
     else
         filename = "???";

     printf("     %d: 0x%lx: %s%s+0x%x (%s)\n", caller_depth, instr_ptr, 
	    procedure_name, ret == -UNW_ENOMEM ? "..." : "",
	    (unsigned int)proced_offset_of_call, filename);
     caller_depth++;
  }

}


static void
print_errno_to_stderr(const char * err_ctx)
{
  int curr_errno = errno;
  char err_descr[2048];

  if (0 == strerror_r(curr_errno, err_descr, sizeof err_descr)) {
     fprintf(stderr, "ERROR: %s: (errno=%d) %s\n",
             err_ctx, curr_errno, err_descr);
  } else {
     /* strerror_r() itself also failed: print a generic error to stderr */
     fprintf(stderr, "ERROR: %s and strerror_r both failed (errnos=%d and %d)\n",
             err_ctx, curr_errno, errno);
  } 
}

static int
record_profiling_info_at_entrance_in_function_call(const char *funct_name)
{
  struct track_calling_stack * new_proc_in_stack;
  new_proc_in_stack = (struct track_calling_stack *) malloc(sizeof(struct track_calling_stack));
  if (!new_proc_in_stack) {
     /* malloc()) failed */
     print_errno_to_stderr("pltenter: malloc");
     return -1;  /* return a negative number: a failure */
  }

  /* Link calling-stack data structure */
  memset((void *)new_proc_in_stack, 0, sizeof(struct track_calling_stack));
  new_proc_in_stack->next_in_stack = thread_caller_stack;

  /* Save procedure name as a validation-check in pltexit at return of it */
  /* TODO 11: solve case when the function in the shared-library is recursive */
  new_proc_in_stack->procedure_name = strdup(funct_name);
  if (!new_proc_in_stack->procedure_name) {
     /* strdup() failed */
     print_errno_to_stderr("pltenter: strdup");
     /* strdup() failed, so no point of tracking this profiling entry */
     /* TODO 11: solve case when the function in the shared-library is recursive */
     free(new_proc_in_stack);
     return -2;  /* return another negative number: another type of failure */
   }

   /* Record profiling data at this entrance to this procedure */
   int r;
   r= getrusage(RUSAGE_THREAD, &(new_proc_in_stack->profiling_data_at_entry_in_call));
   if (r == -1) {
      /* getrusage(RUSAGE_THREAD) failed */
      print_errno_to_stderr("pltenter: getrusage");
      /* getrusage() failed, so no point of tracking this profiling entry */
      /* TODO 11: solve case when the function in the shared-library is recursive */
      free(new_proc_in_stack->procedure_name);
      free(new_proc_in_stack);
      return -3; /* return another negative number: another failure */
   }

   /* getrusage() was successful, so we could save profiling data at entry */
   thread_caller_stack = new_proc_in_stack;
   return 0;   /* return 0, ie., SUCCESS */
}

extern Elf64_Addr 
la_x86_64_gnu_pltenter(Elf64_Sym *__sym, unsigned int __ndx,
                       uintptr_t *__refcook, uintptr_t *__defcook,
                       La_x86_64_regs *__regs, unsigned int *__flags,
                       const char *__symname, long int *__framesizep)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  uint64_t register_x86_64_stack_pointer = __regs->lr_rsp; 
  uint64_t return_address_to_callee = (* (uint64_t *)register_x86_64_stack_pointer);

  printf("%lu.%06lu: %d: Calling symbol %s from address %lx" 
         " in object with cookie %lx defined in object with cookie %lx", 
         tp.tv_sec, tp.tv_usec, kernel_thread_id, __symname, 
         return_address_to_callee, (unsigned long int) __refcook, 
         (unsigned long int) __defcook);

  /* Print x86-64 registers 
   *     The first six integer or pointer arguments are passed in registers 
   *     RDI, RSI, RDX, RCX, R8, and R9.
   * http://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI
   */
  printf(" (RDI=%lu, RSI=%lu, RDX=%lu, RCX=%lu, R8=%lu, R9=%lu)\n", 
         __regs->lr_rdi, __regs->lr_rsi, __regs->lr_rdx, 
         __regs->lr_rcx, __regs->lr_r8, __regs->lr_r9 );

  /* Show the caller stack */
  show_caller_stack_backtrace(__symname, __defcook, __regs);

  /* TODO: Find better approximation for __framesizep, not 4KB */
  *__framesizep = 4096;

  /* save profiling point at the entrance into this procedure */
  record_profiling_info_at_entrance_in_function_call(__symname);

  /* From man page of rtld-audit(7):
   *
   * The return value of la_pltenter() is as for la_symbind*(). 
   */
   return __sym->st_value;
}


static int
calculate_profiling_cost_at_exit_of_this_function_call(const char * ret_func_name) 
{

  /* do we have a stack recorded for this thread */
  if (!thread_caller_stack) {
     fprintf(stderr, "WARNING: calculating_profiling_cost: no stack-structure.\n");
     return -2;  /* return -1 is used by a failing getrusage(3) below */
  }

  /* validation: is the top of our stack expecting the name "ret_func_name" ?
   *    
   *    Warning: see TODO 222 above, for an unusual case 
   *    
   *    Use strncmp( only first 4096 ) instead of strcmp( all chars ) to avoid
   *    case of a misbehaving audited program (which is an arbitrary program 
   *    really) which happened to corrupt the heap memory --in particular, which
   *    happen to corrupt our "thread_caller_stack" data structure
   */

  if (!thread_caller_stack->procedure_name  || 
      0 != strncmp(ret_func_name, thread_caller_stack->procedure_name, 4096)) {
     /* the top of our stack was not expecting the name "ret_func_name" */
     fprintf(stderr, "ERROR: calculating_profiling_cost: "
                     "expected return from function '%s' "
                     "but received return from function '%s' instead\n",
                     thread_caller_stack->procedure_name, ret_func_name);
     /* TODO: should we do anything with the mismatching head of our calling-stack?
      *       see unexpected case described in TODO 222 above */
     return -3;
  }

  /* the validation was ok: the top of our stack is the same as "ret_func_name"
   * which is indeed returning now */

  struct rusage  profiling_data_at_exit;  /* profiling data itself */
  int r;
  r= getrusage(RUSAGE_THREAD, &profiling_data_at_exit);
  if (r == -1) {
     /* getrusage(RUSAGE_THREAD) failed */
     print_errno_to_stderr("pltexit: getrusage");
     /* getrusage() failed, so we can't calculate the cost of this funct-call */
  } else {
     /* getrusage(RUSAGE_THREAD) was successful: calculate cost */
    struct timeval delta_user_time; /* delta in user CPU time */
    struct timeval delta_kern_time; /* delta in kernel CPU time */

    timersub(&profiling_data_at_exit.ru_utime, &(thread_caller_stack->profiling_data_at_entry_in_call.ru_utime), &delta_user_time);
    timersub(&profiling_data_at_exit.ru_stime, &(thread_caller_stack->profiling_data_at_entry_in_call.ru_stime), &delta_kern_time);

    printf("     profiling call of %s:\n", ret_func_name);
    printf("        user-mode time spent: %lu.%06lu\n", delta_user_time.tv_sec, delta_user_time.tv_usec);
    printf("        kernel-mode time spent: %lu.%06lu\n", delta_kern_time.tv_sec, delta_kern_time.tv_usec);
  }

  /* free the current top of our caller-stack */
  struct track_calling_stack * curr_funct_frame;
  curr_funct_frame = thread_caller_stack;
  thread_caller_stack = curr_funct_frame->next_in_stack;

  free(curr_funct_frame->procedure_name);
  free(curr_funct_frame);
  
  return r;  /* return same value as our getrusage() */
}


extern unsigned int 
la_x86_64_gnu_pltexit(Elf64_Sym *__sym, unsigned int __ndx,
                      uintptr_t *__refcook, uintptr_t *__defcook,
                      const La_x86_64_regs *__inregs, 
                      La_x86_64_retval *__outregs, const char *symname)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  pid_t kernel_thread_id = __gettid();  

  printf("%lu.%06lu: %d: Returning from symbol %s from object with cookie %lx"
         " in object with cookie %lx\n", tp.tv_sec, tp.tv_usec, 
         kernel_thread_id, symname, 
	 (unsigned long int) __refcook, (unsigned long int) __defcook);

  /* calculate cost of this procedure call (profiling) */
  calculate_profiling_cost_at_exit_of_this_function_call(symname);

  return (0);
}

