#include "sdb.h"

#include <cpu/cpu.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "memory/vaddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
    static char* line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char* args) {
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char* args) {
    return -1;
}

static int cmd_help(char* args);

static int cmd_si(char* args) {
    char* arg = strtok(args, " ");
    int next = 1;
    if (arg == NULL) {
        next = 1;
    } else {
        next = strtol(arg, NULL, 10);
    }
    if (next < 1) {
        printf("Parse number failed: %s\n", args);
        return 0;
    }
    printf("Execution %d instructions\n", next);
    cpu_exec(next);
    return 0;
}

static int cmd_info(char* args) {
    char* arg = strtok(args, " ");
    if (strcmp(arg, "r") == 0) {
        isa_reg_display();
    } else if (strcmp(arg, "w") == 0) {
        list_wp();
    } else {
        printf("Unknown subcommand: %s\n", args);
    }
    return 0;
}

static int cmd_x(char* args) {
    char* arg0 = strtok(args, " ");
    char* arg1 = strtok(NULL, " ");
    int size = strtol(arg0, NULL, 10);
    vaddr_t offset = strtol(arg1, NULL, 16);
    printf("scan memory: offset = " FMT_WORD ", size = %d\n", offset, size);
    vaddr_t addr = offset;
    for (int i = 0; i < size; i++) {
        word_t value = vaddr_read(addr + i, 1);
        printf("0x%02lx ", (value & 0xff));
        if ((i > 0) && (i & 0xf) == 0) {
            printf("\n");
        }
    }
    if ((size & 0xf) != 0) {
        printf("\n");
    }
    return 0;
}

static int cmd_p(char* args) {
    bool success = false;
    word_t ret = run_expr(args, &success);
    if (!success) {
        printf("Failed to eval %s\n", args);
    } else {
        printf(FMT_WORD "\n", ret);
    }
    return 0;
}

static int cmd_w(char* args) {
    add_wp(args);
    return 0;
}

static int cmd_d(char* args) {
    int no = strtol(args, NULL, 10);
    rem_wp(no);
    return 0;
}

static struct {
    const char* name;
    const char* description;
    int (*handler)(char*);
} cmd_table[] = {
        {"help", "Display informations about all supported commands", cmd_help},
        {"c", "Continue the execution of the program", cmd_c},
        {"si", "Single Instruction Execution", cmd_si},
        {"info", "Show Information", cmd_info},
        {"p", "Print expression", cmd_p},
        {"d", "Delete watchpoint", cmd_d},
        {"w", "Add watchpoint", cmd_w},
        {"x", "Scan Memory", cmd_x},
        {"q", "Exit NEMU", cmd_q}
        /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char* args) {
    /* extract the first argument */
    char* arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() {
    is_batch_mode = true;
}

void sdb_mainloop() {
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    for (char* str; (str = rl_gets()) != NULL;) {
        char* str_end = str + strlen(str);

        /* extract the first token as the command */
        char* cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
     * which may need further parsing
     */
        char* args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}

void init_sdb() {
    /* Compile the regular expressions. */
    init_regex();

    /* Initialize the watchpoint pool. */
    init_wp_pool();

    // test_expr_cases();
}
