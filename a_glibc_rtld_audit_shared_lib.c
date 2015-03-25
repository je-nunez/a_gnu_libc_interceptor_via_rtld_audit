
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

  return __name;
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

  /* From man page of rtld-audit(7):
   *
   * The return value of la_symbind32() and la_symbind64() is the address to 
   * which control should be passed after the function returns. If the auditing 
   * library is simply monitoring symbol bindings, then it should return 
   * sym->st_value. A different value may be returned if the library wishes to 
   * direct control to an alternate location. */
  return __sym->st_value;
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

