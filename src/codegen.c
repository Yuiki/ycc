#include "ycc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int label_cnt = 0;

int align_cnt = 0;

int calc_offset(Type *type) {
  if (type->ty == CHAR) {
    return 1;
  } else if (type->ty == INT) {
    return 4;
  } else if (type->ty == PTR) {
    return 8;
  } else { // array
    return type->array_size * calc_offset(type->ptr_to);
  }
}

int calc_stack_size(LVar *locals) {
  int size = 0;
  for (LVar *local = locals; local; local = local->next) {
    size += calc_offset(local->type);
  }
  return size + ((16 - (size % 16)) % 16);
}

void store(TypeKind type) {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  if (type == CHAR) {
    printf("  mov [rax], dil\n");
  } else if (type == INT) {
    printf("  mov [rax], edi\n");
  } else {
    printf("  mov [rax], rdi\n");
  }
}

void load(TypeKind type) {
  printf("  pop rax\n");
  if (type == CHAR) {
    printf("  movsx rax, BYTE PTR [rax]\n");
  } else if (type == INT) {
    printf("  mov eax, [rax]\n");
  } else {
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
}

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

void gen_num(Node *node) { printf("  push %d\n", node->val); }

void gen_str(Node *node) {
  printf("  lea rax, .LC%d[rip]\n", node->index);
  printf("  push rax\n");
}

void gen_lvar(Node *node) {
  gen_val(node);
  if (node->type->ty != ARRAY) {
    load(node->type->ty);
  }
}

void gen_gvar_decra(Node *node) {
  int size = calc_offset(node->type);
  // TODO: fix alignment
  printf("  .comm %.*s,%d,16\n", node->name_len, node->name, size);
}

void gen_gvar(Node *node) {
  gen_val(node);
  if (node->type->ty != ARRAY) {
    load(node->type->ty);
  }
}

void gen_addr(Node *node) { gen_val(node->lhs); }

void gen_deref(Node *node) {
  gen(node->lhs);
  load(node->type->ty);
}

void gen_func(Node *node) {
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

    store(param->type->ty);

    param = param->next;
  }

  gen(node->block);
  printf("  pop rax\n");

  // epilogue
  // TODO: remove
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_call(Node *node) {
  Node *arg = node->args;
  // TODO: support >6 args
  int i;
  for (i = 0; arg != NULL && i < 6; i++) {
    gen(arg);
    arg = arg->next;
  }
  for (int j = i - 1; j >= 0; j--) {
    if (j == 0) {
      printf("  pop rdi\n");
    } else if (j == 1) {
      printf("  pop rsi\n");
    } else if (j == 2) {
      printf("  pop rdx\n");
    } else if (j == 3) {
      printf("  pop rcx\n");
    } else if (j == 4) {
      printf("  pop r8\n");
    } else if (j == 5) {
      printf("  pop r9\n");
    }
  }

  int label = align_cnt++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n");
  printf("  jne .Abegin%d\n", label);
  printf("  call %.*s\n", node->func_len, node->func);
  printf("  jmp .Aend%d\n", label);
  printf(".Abegin%d:\n", label);
  printf("  sub rsp, 8\n");
  printf("  call %.*s\n", node->func_len, node->func);
  printf("  add rsp, 8\n");
  printf(".Aend%d:\n", label);

  printf("  push rax\n");
}

void gen_assign(Node *node) {
  if (node->lhs->kind == ND_DEREF) {
    gen(node->lhs->lhs);
  } else {
    gen_val(node->lhs);
  }
  gen(node->rhs);

  store(node->lhs->type->ty);
  printf("  push rdi\n");
}

void gen_return(Node *node) {
  gen(node->lhs);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_if(Node *node) {
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
}

void gen_while(Node *node) {
  int label = label_cnt++;
  printf(".Lbegin%d:\n", label);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", label);
  gen(node->then);
  printf("  jmp .Lbegin%d\n", label);
  printf(".Lend%d:\n", label);
}

void gen_for(Node *node) {
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
}

void gen_block(Node *node) {
  Node *head = node->body;
  while (head != NULL) {
    gen(head);
    head = head->next;
  }
}

void gen_child(Node *node) {
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
}

void gen_add(Node *node) {
  gen_child(node);

  if (node->lhs->kind == ND_LVAR) {
    if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) {
      int offset = calc_offset(node->lhs->type->ptr_to);
      printf("  imul rdi, %d\n", offset);
    }
  }
  if (node->rhs->kind == ND_LVAR) {
    if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) {
      int offset = calc_offset(node->rhs->type->ptr_to);
      printf("  imul rax, %d\n", offset);
    }
  }
  printf("  add rax, rdi\n");

  printf("  push rax\n");
}

void gen_sub(Node *node) {
  gen_child(node);

  if (node->lhs->kind == ND_LVAR) {
    if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) {
      int offset = calc_offset(node->lhs->type->ptr_to);
      printf("  imul rdi, %d\n", offset);
    }
  }
  if (node->rhs->kind == ND_LVAR) {
    if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) {
      int offset = calc_offset(node->rhs->type->ptr_to);
      printf("  imul rax, %d\n", offset);
    }
  }
  printf("  sub rax, rdi\n");

  printf("  push rax\n");
}

void gen_mul(Node *node) {
  gen_child(node);
  printf("  imul rax, rdi\n");

  printf("  push rax\n");
}

void gen_div(Node *node) {
  gen_child(node);

  printf("  cqo\n");
  printf("  idiv rdi\n");

  printf("  push rax\n");
}

void gen_eq(Node *node) {
  gen_child(node);

  printf("  cmp rax, rdi\n");
  printf("  sete al\n");
  printf("  movzb rax, al\n");

  printf("  push rax\n");
}

void gen_ne(Node *node) {
  gen_child(node);

  printf("  cmp rax, rdi\n");
  printf("  setne al\n");
  printf("  movzb rax, al\n");

  printf("  push rax\n");
}

void gen_lt(Node *node) {
  gen_child(node);

  printf("  cmp rax, rdi\n");
  printf("  setl al\n");
  printf("  movzb rax, al\n");

  printf("  push rax\n");
}

void gen_le(Node *node) {
  gen_child(node);

  printf("  cmp rax, rdi\n");
  printf("  setle al\n");
  printf("  movzb rax, al\n");

  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    gen_num(node);
    return;
  case ND_STR:
    gen_str(node);
    return;
  case ND_LVAR:
    gen_lvar(node);
    return;
  case ND_GVAR_DECLA:
    gen_gvar_decra(node);
    return;
  case ND_GVAR:
    gen_gvar(node);
    return;
  case ND_ADDR:
    gen_addr(node);
    return;
  case ND_DEREF:
    gen_deref(node);
    return;
  case ND_FUNC:
    gen_func(node);
    return;
  case ND_CALL:
    gen_call(node);
    return;
  case ND_ASSIGN:
    gen_assign(node);
    return;
  case ND_RETURN:
    gen_return(node);
    return;
  case ND_IF:
    gen_if(node);
    return;
  case ND_WHILE:
    gen_while(node);
    return;
  case ND_FOR:
    gen_for(node);
    return;
  case ND_BLOCK:
    gen_block(node);
    return;
  case ND_ADD:
    gen_add(node);
    break;
  case ND_SUB:
    gen_sub(node);
    break;
  case ND_MUL:
    gen_mul(node);
    break;
  case ND_DIV:
    gen_div(node);
    break;
  case ND_EQ:
    gen_eq(node);
    break;
  case ND_NE:
    gen_ne(node);
    break;
  case ND_LT:
    gen_lt(node);
    break;
  case ND_LE:
    gen_le(node);
    break;
  case ND_NOP:
    return;
  default:
    fprintf(stderr, "illegal node [NodeKind: %d]", node->kind);
    exit(1);
  }
}
