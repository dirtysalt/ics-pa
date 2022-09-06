#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
    int NO;
    struct watchpoint* next;

    /* TODO: Add more members if necessary */
    char expr[128];
    word_t result;
} WP;

static int wp_number = 0;
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    }

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
    assert(free_ != NULL);
    WP* ret = free_;
    free_ = free_->next;
    return ret;
}
void free_wp(WP* wp) {
    wp->next = free_;
    free_ = wp;
}

void add_wp(const char* expr) {
    bool success = false;
    char buf[128];
    strcpy(buf, expr);
    word_t result = run_expr(buf, &success);
    if (!success) {
        printf("Add watchpoint failed: %s\n", expr);
        return;
    }

    WP* wp = new_wp();
    strcpy(wp->expr, expr);
    wp->NO = wp_number;
    wp->result = result;
    wp_number += 1;

    wp->next = head;
    head = wp;
}

void rem_wp(int no) {
    WP* prev = NULL;
    WP* now = head;
    while (now) {
        if (now->NO == no) {
            break;
        }
        prev = now;
        now = now->next;
    }
    if (now) {
        if (prev)
            prev->next = now->next;
        else
            head = now->next;
        free_wp(now);
    }
}

void list_wp() {
    WP* now = head;
    while (now) {
        printf("no:%d, expr:%s, value:" FMT_WORD "\n", now->NO, now->expr, now->result);
        now = now->next;
    }
}

bool check_wp() {
    WP* now = head;
    bool trigger = false;
    while (now) {
        bool success = false;
        word_t val = run_expr(now->expr, &success);
        if (val != now->result) {
            printf("Watchpoint. no:%d, expr:%s, old:" FMT_WORD ", now:" FMT_WORD "\n", now->NO, now->expr, now->result,
                   val);
            trigger = true;
        }
        now->result = val;
        now = now->next;
    }
    return trigger;
}
