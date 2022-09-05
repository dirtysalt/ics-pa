#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char* code_format =
        "#include <stdio.h>\n"
        "int main() { "
        "  unsigned result = %s; "
        "  printf(\"%%u\", result); "
        "  return 0; "
        "}";

int pos = 0;

static void gen_expr() {
    int r = rand();
    r = r % 3;
    if (r == 0) {
        int r2 = rand();
        if (r2 < 0) r2 = -r2;
        r2 = r2 % 10 + 1;
        pos += sprintf(buf + pos, "%d", r2);
    } else if (r == 1) {
        buf[pos++] = '(';
        gen_expr();
        buf[pos++] = ')';
    } else if (r == 2) {
        gen_expr();
        int r2 = rand();
        r2 = r2 & 0x3;
        char c = '+';
        if (r2 == 0) c = '+';
        if (r2 == 1) c = '-';
        if (r2 == 2) c = '*';
        // if (r2 == 3) c = '/';
        buf[pos++] = c;
        gen_expr();
    }
}

static void gen_rand_expr() {
    pos = 0;
    gen_expr();
    buf[pos] = 0;
}

int main(int argc, char* argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    int i;
    for (i = 0; i < loop; i++) {
        gen_rand_expr();

        sprintf(code_buf, code_format, buf);

        FILE* fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
        if (ret != 0) continue;

        fp = popen("/tmp/.expr", "r");
        assert(fp != NULL);

        int result;
        int sz = fscanf(fp, "%d", &result);
        (void)sz;
        pclose(fp);

        printf("%u %s\n", result, buf);
    }
    return 0;
}
