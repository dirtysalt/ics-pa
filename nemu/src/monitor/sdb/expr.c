#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <memory/vaddr.h>
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    TK_EQ,
    TK_NEQ,
    TK_AND,
    TK_OR,

    /* TODO: Add more token types */
    TK_NUM,
    TK_NEG,
    TK_DEREF,
    TK_REG,
};

static struct rule {
    const char* regex;
    int token_type;
} rules[] = {

        /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

        {" +", TK_NOTYPE},          // spaces
        {"\\+", '+'},               // plus
        {"==", TK_EQ},              // equal
        {"!=", TK_NEQ},             // not equal,
        {"\\&\\&", TK_AND},         // and,
        {"\\|\\|", TK_OR},          // or
        {"\\-", '-'},               // sub
        {"\\*", '*'},               // mul
        {"\\/", '/'},               // div
        {"0x[0-9A-Fa-f]+", TK_NUM}, // num
        {"[0-9]+", TK_NUM},         // num
        {"\\$[0-9a-z]+", TK_REG},   // reg
        {"\\(", '('},               // (
        {"\\)", ')'}                // )
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

                tokens[nr_token].str[0] = 0;
                switch (rules[i].token_type) {
                case '+':
                case '*':
                case '-':
                case '/':
                case '(':
                case ')': {
                    int type = rules[i].token_type;
                    // negative number.
                    if (type == '-') {
                        bool neg = true;
                        if (nr_token > 0) {
                            int pt = tokens[nr_token - 1].type;
                            if (pt == ')' || pt == TK_NUM) {
                                neg = false;
                            }
                        }
                        if (neg) type = TK_NEG;
                    }
                    if (type == '*') {
                        bool deref = true;
                        if (nr_token > 0) {
                            int pt = tokens[nr_token - 1].type;
                            if (pt == ')' || pt == TK_NUM) {
                                deref = false;
                            }
                        }
                        if (deref) type = TK_DEREF;
                    }

                    tokens[nr_token++].type = type;
                    break;
                }
                case TK_NUM:
                case TK_REG:
                case TK_AND:
                case TK_OR:
                case TK_EQ:
                case TK_NEQ: {
                    int type = rules[i].token_type;
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    *(tokens[nr_token].str + substr_len) = 0;
                    tokens[nr_token].type = type;
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

void print_tokens(const char* e) {
    char buf[1024];
    int pos = 0;
    for (int i = 0; i < nr_token; i++) {
        Token* t = &(tokens[i]);
        if (t->type == TK_NUM || t->type == TK_REG || t->type == TK_AND || t->type == TK_OR || t->type == TK_EQ ||
            t->type == TK_NEQ) {
            pos += sprintf(buf + pos, "%s ", t->str);
        } else if (t->type == TK_NEG) {
            pos += sprintf(buf + pos, "-");
        } else if (t->type == TK_DEREF) {
            pos += sprintf(buf + pos, "*");
        } else {
            pos += sprintf(buf + pos, "%c ", t->type);
        }
    }
    buf[pos] = 0;
    Log("expr = %s, tokens = %s", e, buf);
}

static int priority(int t) {
    if (t == TK_AND || t == TK_OR || t == TK_NEQ || t == TK_EQ) return 0;
    if (t == '+' || t == '-') return 1;
    if (t == '*' || t == '/') return 2;
    return 100;
}

static word_t eval_expr(int p, int q, bool* ok) {
    if (p == q) {
        const char* s = tokens[p].str;
        if (s[0] == '0' && s[1] == 'x') {
            return strtoll(s, NULL, 16);
        } else if (s[0] == '$') {
            bool success = false;
            word_t res = isa_reg_str2val(s + 1, &success);
            if (!success) {
                *ok = false;
                Log("Register %s not found", s);
            }
            return res;
        } else {
            return strtoll(s, NULL, 10);
        }
    }
    if (tokens[p].type == TK_NEG) {
        word_t res = eval_expr(p + 1, q, ok);
        return -res;
    }
    if (tokens[p].type == TK_DEREF) {
        word_t res = eval_expr(p + 1, q, ok);
        word_t data = vaddr_read(res, sizeof(word_t));
        return data;
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
            return eval_expr(p + 1, q - 1, ok);
        }
    }
    int depth = 0;
    int op = -1;
    for (int i = p; i <= q; i++) {
        Token* t = tokens + i;
        if (depth == 0) {
            int new = priority(t->type);
            if (new < 100) {
                if (op == -1)
                    op = i;
                else {
                    int old = priority(tokens[op].type);
                    if (new <= old) op = i;
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
    word_t a = eval_expr(p, op - 1, ok);
    word_t b = eval_expr(op + 1, q, ok);
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
        if (b == 0)
            *ok = false;
        else {
            res = a / b;
        }
        break;
    case TK_AND:
        res = a && b;
        break;
    case TK_OR:
        res = a || b;
        break;
    case TK_EQ:
        res = (a == b);
        break;
    case TK_NEQ:
        res = (a != b);
        break;
    }
    Log("token op type = %c(%d, %s), a = %ld, b = %ld, res = %ld", tokens[op].type, tokens[op].type, tokens[op].str, a,
        b, res);
    return res;
}

word_t run_expr(char* e, bool* success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    *success = true;

    print_tokens(e);

    /* TODO: Insert codes to evaluate the expression. */
    // TODO();
    word_t ans = eval_expr(0, nr_token - 1, success);
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
            {"4 * -(2+3)", -20},
            {"4 * (-5)", -20},
            {"4 * -5", -20},
            {"4 + (1*2)", 6},
            {"*0x80000000", 0x00000297},
            {"$ra", 0},
            {"4 != 5", 1},
            {NULL, 0}};

    for (int i = 0; cases[i].s != NULL; i++) {
        bool success = true;
        char* s = cases[i].s;
        int value = run_expr(s, &success);
        int exp = cases[i].exp;
        if (exp != value) {
            panic("eval expr(%s) -> %d, exp = %d", s, value, exp);
        }
    }
}