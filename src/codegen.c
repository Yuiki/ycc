#include "ycc.h"
#include <stdio.h>

int label_idx = 0;

void gen();

// store the first stack value into the second stack address
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

// load from the stack top address and push it
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
    error("node is not variable [NodeKind: %d]", node->kind);
  }

  if (node->kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
  } else { // gvar
    printf("  lea rax, %.*s[rip]\n", node->name_len, node->name);
    printf("  push rax\n");
  }
}

void gen_num(Node *node) { printf("  push %d\n", node->val); }

void gen_str(Node *node) {
  printf("  lea rax, .LC%d[rip]\n", node->index);

  printf("  push rax\n");
}

void gen_gvar_decra(Node *node) {
  int size = size_of(node->type);
  // TODO: set alignment properly
  printf("  .comm %.*s,%d,16\n", node->name_len, node->name, size);
}

void gen_var(Node *node) {
  gen_val(node);
  if (node->type->kind != ARRAY) {
    load(node->type->kind);
  }
}

void gen_addr(Node *node) { gen_val(node->lhs); }

void gen_deref(Node *node) {
  gen(node->lhs);
  load(node->type->kind);
}

void gen_epilogue() {
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_func(Node *node) {
  printf("%.*s:\n", node->func_len, node->func);

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  int stack_size = calc_stack_size(node->locals);
  printf("  sub rsp, %d\n", stack_size);

  // TODO: support >6 params
  Node *param = node->params;
  for (int i = 0; param != NULL && i < 6; i++, param = param->next) {
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

    store(param->type->kind);
  }

  gen(node->block);

  printf("  pop rax\n");

  // epilogue
  // TODO: remove
  gen_epilogue();
}

void gen_call(Node *node) {
  // TODO: support >6 args
  int var_size = 0;
  for (Node *arg = node->args; arg && var_size < 6;
       arg = arg->next, var_size++) {
    gen(arg);
  }

  for (int i = var_size - 1; i >= 0; i--) {
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
  }

  // fix alignment
  int idx = label_idx++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n");
  printf("  jne .Lbegin%d\n", idx);
  printf("  call %.*s\n", node->func_len, node->func);
  printf("  jmp .Lend%d\n", idx);
  printf(".Lbegin%d:\n", idx);
  printf("  sub rsp, 8\n");
  printf("  call %.*s\n", node->func_len, node->func);
  printf("  add rsp, 8\n");
  printf(".Lend%d:\n", idx);

  printf("  push rax\n");
}

void gen_assign(Node *node) {
  if (node->lhs->kind == ND_DEREF) {
    gen(node->lhs->lhs);
  } else {
    gen_val(node->lhs);
  }

  gen(node->rhs);

  store(node->lhs->type->kind);

  printf("  push rdi\n");
}

void gen_return(Node *node) {
  gen(node->lhs);
  printf("  pop rax\n");
  gen_epilogue();
}

void gen_if(Node *node) {
  int idx = label_idx++;
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lelse%d\n", idx);
  gen(node->then);
  printf("  jmp .Lend%d\n", idx);
  printf(".Lelse%d:\n", idx);
  if (node->els) {
    gen(node->els);
  }
  printf(".Lend%d:\n", idx);
}

void gen_while(Node *node) {
  int idx = label_idx++;
  printf(".Lbegin%d:\n", idx);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", idx);
  gen(node->then);
  printf("  jmp .Lbegin%d\n", idx);
  printf(".Lend%d:\n", idx);
}

void gen_for(Node *node) {
  int idx = label_idx++;
  gen(node->init);
  printf(".Lbegin%d:\n", idx);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", idx);
  gen(node->then);
  gen(node->step);
  printf("  jmp .Lbegin%d\n", idx);
  printf(".Lend%d:\n", idx);
}

void gen_block(Node *node) {
  for (Node *head = node->body; head != NULL; head = head->next) {
    gen(head);
  }
}

// rdi = gen(rhs)
// rax = gen(lhs)
void gen_child(Node *node) {
  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
}

// add offset for add/sub if needed
void gen_ptr_offset(Node *node) {
  Type *ltype = node->lhs->type;
  if (node->lhs->kind == ND_LVAR &&
      (ltype->kind == PTR || ltype->kind == ARRAY)) {
    int offset = size_of(ltype->ptr_to);
    printf("  imul rdi, %d\n", offset);
  }

  Type *rtype = node->rhs->type;
  if (node->rhs->kind == ND_LVAR &&
      (rtype->kind == PTR || rtype->kind == ARRAY)) {
    int offset = size_of(rtype->ptr_to);
    printf("  imul rax, %d\n", offset);
  }
}

void gen_add(Node *node) {
  gen_child(node);

  gen_ptr_offset(node);
  printf("  add rax, rdi\n");

  printf("  push rax\n");
}

void gen_sub(Node *node) {
  gen_child(node);

  gen_ptr_offset(node);
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
  case ND_GVAR_DECLA:
    gen_gvar_decra(node);
    return;
  case ND_GVAR:
  case ND_LVAR:
    gen_var(node);
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
    error("illegal node [NodeKind: %d]\n", node->kind);
  }
}

void gen_program() {
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  // gen strs
  for (Str *str = strs; str; str = str->next) {
    printf(".LC%d:\n", str->index);
    printf("  .string %.*s\n", str->len, str->value);
  }

  // gen globals
  for (int i = 0; globals[i]; i++) {
    gen(globals[i]);
  }
}
