#include "soyocc.h"
#include <stdlib.h>
#include <string.h>

Var *locals;

// 按名字检索变量。找不到返回 NULL
Var *find_var(Token *tok) {
  for (Var *t = locals; t; t = t->next) {
    if (t->len == tok->len && !memcmp(tok->str, t->name, tok->len)) {
      return t;
    }
  }
  return NULL;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = expr;
  return node;
}

/**
 * program    = stmt*
 * stmt       = "return" expr ";" | expr ";"
 *            | "if" "(" expr ")" stmt ("else" stmt)?
 *            | "while" "(" expr ")" stmt
 *            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 * expr       = assign
 * assign     = equality ("=" assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary)*
 * unary      = ("+" | "-")? primary
 * primary    = num | ident | "(" expr ")"
 */

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Program *program() {
  Node head;
  head.next = NULL;
  Node *cur = &head;
  locals = NULL;
  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }
  Program *prog = calloc(1, sizeof(Program));
  prog->node = head.next;
  prog->locals = locals;
  return prog;
}

Node *stmt() {
  Node *node;
  if (consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    consume("(");
    Node *cond = expr();
    node->cond = cond;
    consume(")");
    Node *then_stmt = stmt();
    if (consume("else")) {
      Node *else_stmt = stmt();
      node->then_stmt = then_stmt;
      node->else_stmt = else_stmt;
    } else {
      node->then_stmt = then_stmt;
    }
    return node;
  }
  if (consume("while")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    consume("(");
    node->cond = expr();
    consume(")");
    node->loop_stmt = stmt();
    return node;
  }
  if (consume("for")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    consume("(");

    if (!consume(";")) {
      node->init = expr();
      consume(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      consume(";");
    }
    if (!consume(")")) {
      node->iter = expr();
      consume(")");
    }

    consume(")");
    node->loop_stmt = stmt();
    return node;
  }
  if (consume("return"))
    node = new_unary(ND_RETURN, expr());
  else
    node = new_unary(ND_STMT_EXPR, expr());
  consume(";");
  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_binary(ND_SUB, new_node_num(0), primary());
  return primary();
}

Var *push_var(Token *tok) {
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = tok->str;
  var->len = tok->len;
  locals = var;
  return var;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Token *tok = consume_ident();
  if (tok) {
    Var *var = find_var(tok);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    if (var)
      node->var = var;
    else
      node->var = push_var(tok);
    return node;
  }
  return new_node_num(expect_number());
}
