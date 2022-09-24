#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]);
extern char** environ;
void call_main(uintptr_t* args) {
    int argc = (int)(args[0]);
    char** argv = (char**)(args + 1);
    // argc argv NULL.
    environ = (char**)(args + argc + 2);
    // char* empty[] = {NULL};
    // environ = empty;
    // exit(main(0, empty, empty));
    exit(main(argc, argv, environ));
    assert(0);
}
