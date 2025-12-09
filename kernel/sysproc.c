#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}
struct semaphore *s;
uint64
sys_sem_init(void)
{
  uint64 uaddr;          // user pointer to sem_t
  int pshared;           // ignored in this xv6 implementation
  int value;             // initial value
  struct proc *p = myproc();

  if (argaddr(0, &uaddr) < 0 || argint(1, &pshared) < 0 || argint(2, &value) < 0)
    return -1;

  int idx = semalloc();
  if (idx < 0)
    return -1;

  // initialize the semaphore value
  struct semaphore *s = &semtable.sem[idx];
  acquire(&s->lock);
  if (!s->valid) {   // sanity check
    release(&s->lock);
    return -1;
  }
  s->count = value;
  release(&s->lock);

  sem_t semval = idx;  // user-visible sem_t is just the index

  if (copyout(p->pagetable, uaddr, (char *)&semval, sizeof(semval)) < 0) {
    semdealloc(idx);
    return -1;
  }

  return 0;
}

uint64
sys_sem_wait(void)
{
  uint64 uaddr;       // user pointer to sem_t
  struct proc *p = myproc();
  sem_t idx;

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(idx)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  struct semaphore *s = &semtable.sem[idx];

  acquire(&s->lock);
  if (!s->valid) {
    release(&s->lock);
    return -1;
  }

  // classic counting semaphore wait(P)
  while (s->count == 0) {
    // sleep on the semaphore; sleep releases s->lock and reacquires it on wakeup
    sleep(s, &s->lock);

    if (!s->valid) {    // if it was destroyed while we were asleep
      release(&s->lock);
      return -1;
    }
  }

  s->count--;
  release(&s->lock);
  return 0;
}

uint64
sys_sem_post(void)
{
  uint64 uaddr;       // user pointer to sem_t
  struct proc *p = myproc();
  sem_t idx;

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(idx)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  struct semaphore *s = &semtable.sem[idx];

  acquire(&s->lock);
  if (!s->valid) {
    release(&s->lock);
    return -1;
  }

  // classic semaphore signal(V)
  s->count++;
  wakeup(s);   // wake up any sleepers waiting on this semaphore
  release(&s->lock);

  return 0;
}

uint64
sys_sem_destroy(void)
{
  uint64 uaddr;       // user pointer to sem_t
  struct proc *p = myproc();
  sem_t idx;

  if (argaddr(0, &uaddr) < 0)
    return -1;

  if (copyin(p->pagetable, (char *)&idx, uaddr, sizeof(idx)) < 0)
    return -1;

  if (idx < 0 || idx >= NSEM)
    return -1;

  // mark it free in the kernel table
  semdealloc(idx);

  // optional: you could also write -1 back to *sem in user space
  // to indicate it's destroyed, but the lab doesn’t require it.

  return 0;
}
uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;

  addr = p->sz;
  if (n == 0)
    return addr;

  uint64 new_sz = addr + n;
  if(new_sz < p->sz){
    return (uint64)-1;
  }
  p->sz = new_sz;
  /*old eager allocatoin, we don't call growproc right away for lazy allocatoin*/
  /*if(growproc(n) < 0)
    return -1;*/
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_freepmem(void)
{
  uint64 pages = kfreepages_count();
  return pages * PGSIZE;
}