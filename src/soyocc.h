#include <stdarg.h>
#include <stdbool.h>

//
// tokennize.c
//
// token 的种类
typedef enum {
  TK_RESERVED, // 符号
  TK_IDENT,    // 标识符
  TK_NUM,      // 整数
  TK_EOF,      // 输入结束
} TokenKind;

typedef struct Token Token;
// 解决 Token 结构体中 Token *next 的前向声明问题
// https://zhuanlan.zhihu.com/p/675837704

struct Token {
  TokenKind kind; // token 的类型
  Token *next;    // 下一个输入的 token
  int val;        // kind 为 TK_NUM 时，token 对应的数值
  char *str;      // token 字符串
  int len;        // token 的长度
};

void error_at(char *loc, char *fmt, ...);
bool startswith(char *p, char *q);

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(char *p);

extern char *user_input;
extern Token *token;

//
// parse.c
//

typedef struct Var Var;

// 局部变量的类型
struct Var {
  Var *next; // 用链表存变量
  char *name;
  int len;
  int offset; // 相对 rbp 的偏移量
};

typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_NUM,       // 整数
  ND_VAR,       // 局部变量
  ND_ASSIGN,    // =
  ND_RETURN,    // return
  ND_STMT_EXPR, // 表达式语句
  ND_IF,        // if
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *next; // 用链表存储 statements
  Node *lhs;
  Node *rhs;
  int val;
  Var *var; // kind == ND_VAR 时，变量信息
  // if
  Node *cond;
  Node *then_stmt;
  Node *else_stmt;
};

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_unary(NodeKind kind, Node *expr);
Node *new_node_num(int val);

typedef struct {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

extern Var *locals;
Program *program();

//
// codegen.c
//

void codegen(Program *prog);
