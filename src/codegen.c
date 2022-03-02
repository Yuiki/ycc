#include "ycc.h"
#include <stdio.h>
#include <stdlib.h>

int label_cnt = 0;

void gen_val(Node *node) {
  if (node->kind != ND_LVAR && node->kind != ND_GVAR) {
    error("代入の左辺値が変数ではありません");
  }

  if (node->kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
  } else {
    printf("  lea rax, %.*s[rip]\n", node->name_len, node->name);
    printf("  push rax\n");
  }
}

int calc_stack_size(LVar *locals) {
  int size = 0;
  for (LVar *local = locals; local; local = local->next) {
    int len = 1;
    if (local->type->ty == ARRAY) {
      len = local->type->array_size;
    }
    size += len * 8;
  }
  return size;
}

int calc_global_size(Type *type) {
  if (type->ty == INT) {
    return 4;
  } else if (type->ty == PTR) { // pointer
    return 8;
  } else { // arr
    return type->array_size * calc_global_size(type->ptr_to);
  }
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_ASSIGN:
    if (node->lhs->kind == ND_DEREF) {
      gen(node->lhs->lhs);
    } else {
      gen_val(node->lhs);
    }
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_LVAR:
    gen_val(node);
    if (node->type->ty != ARRAY) {
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
    }
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF: {
    int label = label_cnt++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%d\n", label);
    gen(node->then);
    printf("  jmp .Lend%d\n", label);
    printf(".Lelse%d:\n", label);
    gen(node->els);
    printf(".Lend%d:\n", label);
    return;
  }
  case ND_WHILE: {
    int label = label_cnt++;
    printf(".Lbegin%d:\n", label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->then);
    printf("  jmp .Lbegin%d\n", label);
    printf(".Lend%d:\n", label);
    return;
  }
  case ND_FOR: {
    int label = label_cnt++;
    gen(node->init);
    printf(".Lbegin%d:\n", label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", label);
    gen(node->then);
    gen(node->step);
    printf("  jmp .Lbegin%d\n", label);
    printf(".Lend%d:\n", label);
    return;
  }
  case ND_BLOCK: {
    Node *head = node->body;
    while (head != NULL) {
      gen(head);
      head = head->next;
    }
    return;
  }
  case ND_CALL: {
    Node *arg = node->args;
    // TODO: support >6 args
    for (int i = 0; arg != NULL && i < 6; i++) {
      gen(arg);
      if (i == 0) {
        printf("  pop rdi\n");
      } else if (i == 1) {
        printf("  pop rsi\n");
      } else if (i == 2) {
        printf("  pop rdx\n");
      } else if (i == 3) {
        printf("  pop rcx\n");
      } else if (i == 4) {
        printf("  pop r8\n");
      } else if (i == 5) {
        printf("  pop r9\n");
      }
      arg = arg->next;
    }
    // TODO: support /16 alignment
    printf("  call %.*s\n", node->func_len, node->func);
    printf("  push rax\n");
    return;
  }
  case ND_FUNC: {
    printf("%.*s:\n", node->func_len, node->func);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    int stack_size = calc_stack_size(node->locals);
    printf("  sub rsp, %d\n", stack_size);

    Node *param = node->params;
    // TODO: support >6 params
    for (int i = 0; param != NULL && i < 6; i++) {
      gen_val(param);
      if (i == 0) {
        printf("  push rdi\n");
      } else if (i == 1) {
        printf("  push rsi\n");
      } else if (i == 2) {
        printf("  push rdx\n");
      } else if (i == 3) {
        printf("  push rcx\n");
      } else if (i == 4) {
        printf("  push r8\n");
      } else if (i == 5) {
        printf("  push r9\n");
      }

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");

      param = param->next;
    }

    gen(node->block);
    printf("  pop rax\n");

    // epilogue
    // TODO: remove
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  case ND_ADDR:
    gen_val(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_NOP:
    return;
  case ND_GVAR: {
    gen_val(node);
    if (node->type->ty != ARRAY) {
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
    }
    return;
  }
  case ND_GVAR_DECLA: {
    int size = calc_global_size(node->type);
    // TODO: fix alignment
    printf("  .comm %.*s,%d,16\n", node->name_len, node->name, size);
    return;
  }
  default:
    break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    if (node->lhs->kind == ND_LVAR) {
      if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) {
        if (node->lhs->type->ptr_to->ty == INT) {
          printf("  imul rdi, 4\n");
        } else {
          printf("  imul rdi, 8\n");
        }
      }
    }
    if (node->rhs->kind == ND_LVAR) {
      if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) {
        if (node->rhs->type->ptr_to->ty == INT) {
          printf("  imul rax, 4\n");
        } else {
          printf("  imul rax, 8\n");
        }
      }
    }
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    if (node->lhs->kind == ND_LVAR) {
      if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) {
        if (node->lhs->type->ptr_to->ty == INT) {
          printf("  imul rdi, 4\n");
        } else {
          printf("  imul rdi, 8\n");
        }
      }
    }
    if (node->rhs->kind == ND_LVAR) {
      if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) {
        if (node->rhs->type->ptr_to->ty == INT) {
          printf("  imul rax, 4\n");
        } else {
          printf("  imul rax, 8\n");
        }
      }
    }
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
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
  default:
    fprintf(stderr, "illegal state");
    exit(1);
  }

  printf("  push rax\n");
}
