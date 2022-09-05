#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,

    /* TODO: Add more token types */
    TK_NUM,
};

static struct rule {
    const char* regex;
    int token_type;
} rules[] = {

        /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

        {" +", TK_NOTYPE},  // spaces
        {"\\+", '+'},       // plus
        {"==", TK_EQ},      // equal
        {"\\-", '-'},       // sub
        {"\\*", '*'},       // mul
        {"\\/", '/'},       // div
        {"[0-9]+", TK_NUM}, // num
        {"\\(", '('},       // (
        {"\\)", ')'}        // )
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
} Token;

#define MAX_TOKENS 10240
static Token tokens[MAX_TOKENS] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char* e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char* substr_start = e + position;
                int substr_len = pmatch.rm_eo;

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position,
                    substr_len, substr_len, substr_start);

                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

                switch (rules[i].token_type) {
                case '+':
                case '-':
                case '*':
                case '/':
                case '(':
                case ')': {
                    tokens[nr_token++].type = rules[i].token_type;
                    break;
                }
                case TK_NUM: {
                    // to support negative number.
                    bool neg = false;
                    if (nr_token >= 2) {
                        Token* a = &tokens[nr_token - 1];
                        Token* b = &tokens[nr_token - 2];
                        if (a->type == '-') {
                            if (b->type == TK_NUM || b->type == ')') {
                                neg = false;
                            } else {
                                neg = true;
                            }
                        }
                    }
                    if (neg) {
                        nr_token -= 1;
                        tokens[nr_token].str[0] = '-';
                        strncpy(tokens[nr_token].str + 1, substr_start, substr_len);
                        *(tokens[nr_token].str + substr_len + 1) = 0;
                    } else {
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        *(tokens[nr_token].str + substr_len) = 0;
                    }
                    tokens[nr_token].type = TK_NUM;
                    nr_token++;
                    break;
                }

                case TK_NOTYPE:
                default:
                    break;
                }
                break;
            }
        }
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }

    return true;
}

void print_tokens() {
    char buf[1024];
    int pos = 0;
    for (int i = 0; i < nr_token; i++) {
        Token* t = &(tokens[i]);
        if (t->type == TK_NUM) {
            pos += sprintf(buf + pos, "%s ", t->str);
        } else {
            pos += sprintf(buf + pos, "%c ", t->type);
        }
    }
    buf[pos] = 0;
    Log("tokens = %s", buf);
}

static word_t eval_expr(int p, int q) {
    if (p == q) {
        return strtoll(tokens[p].str, NULL, 10);
    }
    if (tokens[p].type == '(' && tokens[q].type == ')') {
        int depth = 0;
        bool match = true;
        for (int i = p + 1; i <= q - 1; i++) {
            Token* t = tokens + i;
            if (t->type == '(') {
                depth += 1;
            } else if (t->type == ')') {
                depth -= 1;
                if (depth < 0) {
                    match = false;
                    break;
                }
            }
        }
        if (match) {
            return eval_expr(p + 1, q - 1);
        }
    }
    int depth = 0;
    int op = -1;
    for (int i = p; i <= q; i++) {
        Token* t = tokens + i;
        if (depth == 0) {
            if (t->type == '+' || t->type == '-') {
                op = i;
            } else if (t->type == '*' || t->type == '/') {
                if (op == -1 || tokens[op].type == '*' || tokens[op].type == '/') {
                    op = i;
                }
            }
        }
        if (t->type == '(') {
            depth += 1;
        } else if (t->type == ')') {
            depth -= 1;
        }
    }
    if (!(op >= p && op <= q)) {
        panic("eval: op = %d, p = %d, q = %d", op, p, q);
    }
    word_t a = eval_expr(p, op - 1);
    word_t b = eval_expr(op + 1, q);
    word_t res = 0;
    switch (tokens[op].type) {
    case '+':
        res = a + b;
        break;
    case '-':
        res = a - b;
        break;
    case '*':
        res = a * b;
        break;
    case '/':
        res = a / b;
        break;
    }
    Log("token op type = %c, a = %ld, b = %ld, res = %ld", tokens[op].type, a, b, res);
    return res;
}

word_t expr(char* e, bool* success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    print_tokens();

    /* TODO: Insert codes to evaluate the expression. */
    // TODO();
    word_t ans = eval_expr(0, nr_token - 1);
    return ans;
}

void test_expr_cases() {
    struct Case {
        char* s;
        int exp;
    } cases[] = {
            {"4*(((((1)*(2)+8)))*5)*((2))*8*4+((7)*1+4+3*(((3)))+(9)-((5)-(4))+(((6)))*(7)-10+(1+9)+2*(5*(((((5)))))"
             "*8-("
             "3+4)+1)-((8)*((7+(10))))+(9))*(4+2)+((3-(3)*9))+(1)*(1)",
             14763},
            {"(1)*(2)+8", 10},
            {"4 * (-5)", -20},
            {"4 * -5", -20},
            {"4 + (1*2)", 6},
            {NULL, 0}};

    for (int i = 0; cases[i].s != NULL; i++) {
        bool success = true;
        char* s = cases[i].s;
        int value = expr(s, &success);
        int exp = cases[i].exp;
        if (exp != value) {
            panic("eval expr(%s) -> %d, exp = %d", s, value, exp);
        }
    }
}