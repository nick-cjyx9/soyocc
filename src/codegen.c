#include "soyocc.h"
#include <stdio.h>

int labelseq = 1;

char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
  for (Node *t = prog->node; t; t = t->next)
    gen(t);

  // Epilogue
  printf(".Lreturn:\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_VAR)
    error_at(&token->str[0], "not an lvalue");
  printf("  lea rax, [rbp-%d]\n", node->var->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_STMT_EXPR:
    gen(node->lhs);
    // 只执行表达式的副作用，不需要保留表达式的计算结果。
    // 调整栈指针丢弃计算结果
    printf("  add rsp, 8\n");
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  jmp .Lreturn\n");
    return;
  case ND_VAR:
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
  case ND_IF: {
    int seq = labelseq++;
    if (node->else_stmt) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L.else.%d\n", seq);
      gen(node->then_stmt);
      printf("  jmp .L.end.%d\n", seq);
      printf(".L.else.%d:\n", seq);
      gen(node->else_stmt);
      printf(".L.end.%d:\n", seq);
    } else {
      int seq = labelseq;
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L.end.%d\n", seq);
      gen(node->then_stmt);
      printf(".L.end.%d:\n", seq);
    }
    return;
  }
  case ND_WHILE: {
    int seq = labelseq++;
    if (node->init)
      gen(node->init);
    printf(".L.begin.%d:\n", seq);
    if (node->cond && node->is_do_while == false) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L.end.%d\n", seq);
    }
    gen(node->loop_stmt);
    if (node->iter)
      gen(node->iter);
    if (node->cond && node->is_do_while == true) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L.end.%d\n", seq);
    }
    printf("  jmp .L.begin.%d\n", seq);
    printf(".L.end.%d:\n", seq);
    return;
  }
  case ND_BLOCK:
    for (Node *n = node->lhs; n; n = n->next)
      gen(n);
    return;
  case ND_FUNC_CALL: {
    // https://x86-64-assembly.readthedocs.io/zh-cn/latest/chapter_04_subroutines/01_calling_conventions.html
    int n = 0;
    for (Node *arg = node->args; arg; arg = arg->next, n++)
      gen(arg);
    if (n > 6)
      error_at(&token->str[0], "目前只支持最多 6 个参数");
    for (int i = n - 1; i >= 0; i--)
      // 正序建链表，顺序入栈，后进先出
      printf("  pop %s\n", argreg[i]);
    /**
     * System V ABI 规定 rsp 在 call 之前必须是 16 字节对齐的
     * 由于 call 会压入 8 字节的返回地址
     * 所以在 call 之前 rsp 必须是 8 的倍数，这里做动态调整和恢复
     * TODO 通过编译器维护一个栈深度计数器来计算补齐，避免动态调整
     */
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n");
    // 15 = 0b1111
    printf("  jz .L.call.rsp_aligned\n");
    printf("  sub rsp, 8\n");
    printf("  call %s\n", node->func_name);
    printf("  add rsp, 8\n");
    printf("  jmp .L.call.done\n");
    printf(".L.call.rsp_aligned:\n");
    printf("  call %s\n", node->func_name);
    printf(".L.call.done\n");
    printf("  push rax\n");
    return;
  }
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
