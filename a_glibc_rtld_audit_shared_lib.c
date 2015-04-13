
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stdio.h>
#include <link.h>
#include <elf.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

/* UNW_LOCAL_ONLY: Unwind caller stack-trace of local architecture only, not 
 * cross-platform */

#define UNW_LOCAL_ONLY

#include <libunwind.h>
#include <dlfcn.h>

#define MAX_PROCEDURE_NAME_LENGTH  4096
 
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
  unw_word_t    proced_offset_of_call ;
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

     printf("     %d: 0x%x: %s%s+0x%x (%s)\n", caller_depth, instr_ptr, 
	    procedure_name, ret == -UNW_ENOMEM ? "..." : "",
	    (unsigned int)proced_offset_of_call, filename);
     caller_depth++;
  }

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
  *__framesizep = 4096 ;
  /* From man page of rtld-audit(7):
   *
   * The return value of la_pltenter() is as for la_symbind*(). 
   */
   return __sym->st_value;
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
  return (0);
}

