#include "ycc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local vars
LVar *locals;

Str *strs;

Token *token;

Node *globals[100];

// return true if the next token is `op`
// Otherwise, return false
bool is_next(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  return true;
}

// Advance `token` and return true if the next token is `op`
// Otherwise, return false
bool consume(char *op) {
  bool found = is_next(op);
  if (found) {
    token = token->next;
  }
  return found;
}

// Advance `token` and return true if the next token is identifier
// Otherwise, return false
Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *curr = token;
  token = token->next;
  return curr;
}

// Advance `token` if the next token is `op`
// Otherwise, occur an error.
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    error_at(token->str, "'%s'ではありません", op);
  }
  token = token->next;
}

// Advance `token` if the next token is number.
// Otherwise, occur an error.
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

Type *new_type(TypeKind ty) {
  Type *type = calloc(1, sizeof(Type));
  type->ty = ty;
  return type;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  if (lhs->type->ty == PTR || rhs->type->ty == PTR || lhs->type->ty == ARRAY ||
      rhs->type->ty == ARRAY) {
    Type *new_ty = new_type(PTR);
    Type *ptr_to;
    if (lhs->type->ptr_to != NULL) {
      ptr_to = lhs->type->ptr_to;
    } else {
      ptr_to = rhs->type->ptr_to;
    }
    new_ty->ptr_to = ptr_to;
    node->type = new_ty;
  } else { // int or char
    if (lhs->type->ty == PTR || lhs->type->ty == ARRAY) {
      node->type = new_type(rhs->type->ty);
    } else {
      node->type = new_type(lhs->type->ty);
    }
  }

  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = new_type(INT);
  return node;
}

Type *find_gvar(Token *tok) {
  for (int i = 0; globals[i]; i++) {
    Node *global = globals[i];
    if (global->kind == ND_GVAR_DECLA && global->name_len == tok->len &&
        !memcmp(tok->str, global->name, global->name_len)) {
      return global->type;
    }
  }
  return false;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Node *expr();

Node *create_var(Token *tok, Type *type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->type = type;

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    node->offset = lvar->offset;
  } else {
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals) {
      lvar->offset = locals->offset + 8;
    } else {
      lvar->offset = 8;
    }
    node->offset = lvar->offset;
    locals = lvar;
  }
  lvar->type = type;

  return node;
}

int create_str(char *value, int len) {
  Str *str = calloc(1, sizeof(Str));
  str->next = strs;
  str->value = value;
  str->len = len;
  if (strs) {
    str->index = strs->index + 1;
  } else {
    str->index = 0;
  }
  strs = str;

  return str->index;
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) { // function call
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->func = tok->str;
      node->func_len = tok->len;
      // TODO: set proper type
      node->type = new_type(INT);

      Node *head = NULL;
      while (!consume(")")) {
        if (head == NULL) {
          head = expr();
          node->args = head;
        } else {
          head->next = expr();
          head = head->next;
        }
        if (!consume(",")) {
          expect(")");
          break;
        }
      }
      return node;
    } else { // var
      Node *node = calloc(1, sizeof(Node));
      LVar *lvar = find_lvar(tok);
      if (lvar) {
        node->kind = ND_LVAR;

        node->offset = lvar->offset;
        node->type = lvar->type;

        if (consume("[")) {
          Node *add = new_node(ND_ADD, node, expr());

          Node *deref = calloc(1, sizeof(Node));
          deref->kind = ND_DEREF;
          deref->lhs = add;
          deref->type = new_type(node->type->ty);

          expect("]");

          return deref;
        }
        return node;
      }

      Type *type = find_gvar(tok);
      if (type) {
        node->kind = ND_GVAR;
        node->name = tok->str;
        node->name_len = tok->len;
        node->type = type;

        if (consume("[")) {
          Node *add = new_node(ND_ADD, node, expr());

          Node *deref = calloc(1, sizeof(Node));
          deref->kind = ND_DEREF;
          deref->lhs = add;
          deref->type = new_type(node->type->ty);

          expect("]");

          return deref;
        }
        return node;
      } else {
        error_at(token->str, "未定義の識別子です");
      }
    }
  }

  if (token->kind == TK_STR_LIT) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_STR;
    node->type = new_type(ARRAY);
    node->type->array_size = token->len;
    node->type->ptr_to = new_type(CHAR);

    node->index = create_str(token->str, token->len);

    token = token->next;

    return node;
  }

  return new_node_num(expect_number());
}

int byte_of(TypeKind type) {
  if (type == CHAR) {
    return 1;
  } else if (type == INT) {
    return 4;
  } else { // pointer
    return 8;
  }
}

Node *unary() {
  if (token->kind == TK_SIZEOF) {
    token = token->next;

    Node *child = unary();
    return new_node_num(byte_of(child->type->ty));
  }
  if (consume("*")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->lhs = unary();
    node->type = node->lhs->type->ptr_to;
    return node;
  }
  if (consume("&")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_ADDR;
    node->lhs = unary();
    node->type = new_type(PTR);
    return node;
  }

  if (consume("+")) {
    return primary();
  }
  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

Node *expr() { return assign(); }

Node *stmt();

Node *block() {
  expect("{");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;

  Node *head = NULL;
  while (!consume("}")) {
    if (head == NULL) {
      head = stmt();
      node->body = head;
      continue;
    }
    head->next = stmt();
    head = head->next;
  }

  return node;
}

TypeKind type_of(Token *token) {
  if (token->kind == TK_CHAR) {
    return CHAR;
  }
  if (token->kind == TK_INT) {
    return INT;
  }
  return -1;
}

bool is_type(Token *token) { return type_of(token) != -1; }

Type *expect_type() {
  TypeKind ty_kind = type_of(token);
  if (ty_kind == -1) {
    error_at(token->str, "型が未定義です");
  }
  token = token->next;

  Type *type = calloc(1, sizeof(Type));
  type->ty = ty_kind;
  while (consume("*")) {
    Type *new_type = calloc(1, sizeof(Type));
    new_type->ptr_to = type;
    new_type->ty = PTR;
    type = new_type;
  }

  return type;
}

Type *type_ident(Token **ident) {
  Type *type = expect_type();

  *ident = consume_ident();
  if (*ident == NULL) {
    error_at(token->str, "識別子ではありません");
  }
  return type;
}

Node *var_decla() {
  Token *ident;
  Type *type = type_ident(&ident);
  if (consume("[")) {
    Type *new_ty = new_type(ARRAY);
    new_ty->array_size = expect_number();
    expect("]");

    new_ty->ptr_to = type;
    type = new_ty;
  }

  create_var(ident, type);

  expect(";");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NOP;
  return node;
}

Node *stmt() {
  Node *node;

  if (token->kind == TK_RETURN) {
    token = token->next;

    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (token->kind == TK_IF) {
    token = token->next;

    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    if (token->kind == TK_ELSE) {
      token = token->next;
      node->els = stmt();
    }
  } else if (token->kind == TK_WHILE) {
    token = token->next;

    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
  } else if (token->kind == TK_FOR) {
    token = token->next;

    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;

    expect("(");

    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }

    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }

    if (!consume(")")) {
      node->step = expr();
      expect(")");
    }

    node->then = stmt();
  } else if (is_next("{")) {
    node = block();
  } else if (is_type(token)) { // declaration
    node = var_decla();
  } else {
    node = expr();
    expect(";");
  }

  return node;
}

Node *global_var(Token *ident, Type *type) {
  if (consume("[")) {
    Type *new_ty = new_type(ARRAY);
    new_ty->array_size = expect_number();
    expect("]");

    new_ty->ptr_to = type;
    type = new_ty;
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_GVAR_DECLA;
  node->type = type;

  node->name = ident->str;
  node->name_len = ident->len;

  expect(";");

  return node;
}

Node *function(Token *ident) {
  locals = NULL;

  expect("(");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->func = ident->str;
  node->func_len = ident->len;

  Node *head = NULL;
  while (!consume(")")) {
    Type *type = expect_type();

    Token *param = consume_ident();
    if (head == NULL) {
      head = create_var(param, type);
      node->params = head;
    } else {
      head->next = create_var(param, type);
      head = head->next;
    }
    if (!consume(",")) {
      expect(")");
      break;
    }
  }

  node->block = block();

  node->locals = locals;

  return node;
}

Node *global() {
  Token *ident;
  Type *type = type_ident(&ident);

  if (is_next("(")) { // function
    return function(ident);
  } else { // global var
    return global_var(ident, type);
  }
}

void program() {
  int i = 0;
  while (!at_eof()) {
    globals[i++] = global();
  }
  globals[i] = NULL;
}
