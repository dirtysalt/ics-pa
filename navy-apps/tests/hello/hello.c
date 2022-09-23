#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

void yield() {
_syscall_(SYS_yield, 0, 0, 0);
}

int main() {
  int ret = write(1, "Hello World!\n", 13);
  if (ret != 13) {
     write(1, "Damn\n", 5);
     return 0;
  }
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
      yield();
    }
  }
  return 0;
}
