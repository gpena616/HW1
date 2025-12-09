#include "param.h"

struct spinlock {
  uint locked;
  char *name;
  struct cpu *cpu;
};

struct semaphore {
  struct spinlock lock;
  int count;
  int valid;
};

struct semtab {
  struct spinlock lock;
  struct semaphore sem[NSEM];
};

extern struct semtab semtable;