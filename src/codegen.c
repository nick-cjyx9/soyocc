#include "soyocc.h"
#include <stdio.h>

void gen(Node *node);

void codegen(Program *prog) {
  // 计算每个局部变量的偏移量和栈帧大小
  int stack_size = 0;
  for (Var *v = prog->locals; v; v = v->next) {
    stack_size += 8;
    v->offset = stack_size;
  }
  prog->stack_size = stack_size;

  // 开始生成汇编
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  // Prologue
  // 确保 26 个变量的区域
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);
  for (Node *t = prog->node; t; t = t->next) {
    gen(t);
    // 作为式子的求值结果，栈上应该留下一个值，
    // 所以把它弹出，以免栈溢出
    printf("  pop rax\n");
  }

  // Epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error_at(&token->str[0], "左值不是变量");
  printf("  lea rax, [rbp-%d]\n", node->var->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    // cqo: 把 RAX 符号扩展为 128 位并 store 到 RDX:RAX
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}
