#include "ycc.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// local vars
LVar *locals;

Str *strs;

Node *globals[100];

Node *expr();

Node *stmt();

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
  type->kind = ty;
  return type;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  if (lhs->type->kind == PTR || rhs->type->kind == PTR ||
      lhs->type->kind == ARRAY || rhs->type->kind == ARRAY) {
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
    if (lhs->type->kind == PTR || lhs->type->kind == ARRAY) {
      node->type = new_type(rhs->type->kind);
    } else {
      node->type = new_type(lhs->type->kind);
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

TypeKind type_of(Token *token) {
  if (is_next("char")) {
    return CHAR;
  }
  if (is_next("int")) {
    return INT;
  }
  return -1;
}

bool is_type(Token *token) { return type_of(token) != -1; }

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
    int diff = size_of(type);
    if (locals) {
      lvar->offset = locals->offset + diff;
    } else {
      lvar->offset = diff;
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

// "(" expr ")" | ident ("(" ")")? | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *ident = consume_ident();
  if (ident) {
    if (consume("(")) { // function call
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->func = ident->str;
      node->func_len = ident->len;
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
      LVar *lvar = find_lvar(ident);
      if (lvar) {
        node->kind = ND_LVAR;

        node->offset = lvar->offset;
        node->type = lvar->type;

        if (consume("[")) {
          Node *add = new_node(ND_ADD, node, expr());

          Node *deref = calloc(1, sizeof(Node));
          deref->kind = ND_DEREF;
          deref->lhs = add;
          deref->type = new_type(add->type->ptr_to->kind);

          expect("]");

          return deref;
        }
        return node;
      }

      Type *type = find_gvar(ident);
      if (type) {
        node->kind = ND_GVAR;
        node->name = ident->str;
        node->name_len = ident->len;
        node->type = type;

        if (consume("[")) {
          Node *add = new_node(ND_ADD, node, expr());

          Node *deref = calloc(1, sizeof(Node));
          deref->kind = ND_DEREF;
          deref->lhs = add;
          deref->type = new_type(node->type->kind);

          expect("]");

          return deref;
        }
        return node;
      } else {
        error_at(token->str, "未定義の識別子です");
      }
    }
  }

  if (token->kind == TK_STR) {
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

// "sizeof" unary | ("+" | "-")? primary | ("*" | "&") unary
Node *unary() {
  if (consume("sizeof")) {
    Node *child = unary();
    return new_node_num(size_of(child->type));
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

// unary ("*" unary | "/" unary)*
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

// mul ("+" mul | "-" mul)*
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

// add ("<" add | "<=" add | ">" add | ">=" add)*
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

// relational ("==" relational | "!=" relational)*
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

// equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

// assign
Node *expr() { return assign(); }

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

Type *expect_type() {
  TypeKind ty_kind = type_of(token);
  if (ty_kind == -1) {
    error_at(token->str, "型が未定義です");
  }
  token = token->next;

  Type *type = calloc(1, sizeof(Type));
  type->kind = ty_kind;
  while (consume("*")) {
    Type *new_type = calloc(1, sizeof(Type));
    new_type->ptr_to = type;
    new_type->kind = PTR;
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

// expr ";"
// | "return" expr ";"
// | "if" "(" expr ")" stmt ("else" stmt)?
// | "while" "(" expr ")" stmt
// | "for" "(" expr? ";" expr? ";" expr? ")" stmt
// | "{" stmt* "}"
Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    if (consume("else")) {
      node->els = stmt();
    }
  } else if (consume("while")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
  } else if (consume("for")) {
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

// stmt*
void program() {
  int i = 0;
  while (!at_eof()) {
    globals[i++] = global();
  }
  globals[i] = NULL;
}
