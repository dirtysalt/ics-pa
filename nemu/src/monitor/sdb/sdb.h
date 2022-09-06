#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

void sdb_mainloop();
word_t run_expr(char* e, bool* success);

void test_expr_cases();

void add_wp(const char* expr);
void rem_wp(int no);
void list_wp();
bool check_wp();
#endif
