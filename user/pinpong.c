#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int p[2];
  char buf[1];

  pipe(p);

  if (fork() == 0) {
    // child
    read(p[0], buf, 1);
    printf("%d: received ping\n", getpid());
    write(p[1], "x", 1);
    exit(0);
  } else {
    // parent
    write(p[1], "x", 1);
    read(p[0], buf, 1);
    printf("%d: received pong\n", getpid());
    wait(0);
  }
  exit(0);
}
