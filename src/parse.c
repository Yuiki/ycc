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

Type *new_type(TypeKind ty) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = ty;
  return type;
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
    error_at(token->str, "it is not '%s'", op);
  }
  token = token->next;
}

// Advance `token` if the next token is number.
// Otherwise, occur an error.
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "it is not number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

Type *expect_type() {
  TypeKind ty_kind = type_of(token);
  if (ty_kind == -1) {
    error_at(token->str, "unknown type");
  }
  token = token->next;

  Type *type = new_type(ty_kind);
  while (consume("*")) {
    Type *new_ty = new_type(PTR);
    new_ty->ptr_to = type;
    type = new_ty;
  }

  return type;
}

bool at_eof() { return token->kind == TK_EOF; }

Node *new_node(NodeKind kind, Type *type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->type = type;
  return node;
}

Node *new_node_child(NodeKind kind, Node *lhs, Node *rhs) {
  Type *type;
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
    type = new_ty;
  } else { // int or char
    if (lhs->type->kind == PTR || lhs->type->kind == ARRAY) {
      type = new_type(rhs->type->kind);
    } else {
      type = new_type(lhs->type->kind);
    }
  }

  Node *node = new_node(kind, type);
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

Node *new_node_num(int val) {
  Node *node = new_node(ND_NUM, new_type(INT));
  node->val = val;
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
  return NULL;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Node *create_var(Token *tok, Type *type) {
  Node *node = new_node(ND_LVAR, type);

  LVar *lvar = find_lvar(tok);
  if (lvar) {
    error_at(token->str, "変数が定義済みです");
  }

  lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;

  int diff = size_of(type);
  if (locals) {
    lvar->offset = locals->offset + diff;
  } else {
    lvar->offset = diff;
  }

  locals = lvar;

  node->offset = lvar->offset;

  return node;
}

// return str index
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

// type ident
Type *type_ident(Token **ident) {
  Type *type = expect_type();

  *ident = consume_ident();
  if (*ident == NULL) {
    error_at(token->str, "it is not identifier");
  }
  return type;
}

// "[" num "]""
Type *consume_array_decla(Type *base_ty) {
  if (!consume("[")) {
    return NULL;
  }
  Type *new_ty = new_type(ARRAY);
  new_ty->array_size = expect_number();
  expect("]");

  new_ty->ptr_to = base_ty;
  return new_ty;
}

// type-ident (array-decla)? ";"
Node *var_decla() {
  Token *ident;
  Type *type = type_ident(&ident);

  Type *arr_type;
  if ((arr_type = consume_array_decla(type))) {
    type = arr_type;
  }

  create_var(ident, type);

  expect(";");

  return new_node(ND_NOP, NULL);
}

// ")" | expr ("," expr)* ")"
Node *func_call(Token *name) {
  // TODO: set proper type
  Node *node = new_node(ND_CALL, new_type(INT));
  node->func = name->str;
  node->func_len = name->len;

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
}

// ("[" expr "]")?
Node *array(Node *base) {
  if (consume("[")) {
    Node *add = new_node_child(ND_ADD, base, expr());
    Node *deref = new_node(ND_DEREF, add->type->ptr_to);
    deref->lhs = add;

    expect("]");

    return deref;
  }
  return NULL;
}

Node *lvar(LVar *lv) {
  Node *node = new_node(ND_LVAR, lv->type);
  node->offset = lv->offset;

  Node *arr;
  if ((arr = array(node))) {
    return arr;
  }
  return node;
}

Node *gvar(Token *name, Type *type) {
  Node *node = new_node(ND_GVAR, type);
  node->name = name->str;
  node->name_len = name->len;

  Node *arr;
  if ((arr = array(node))) {
    return arr;
  }
  return node;
}

Node *var(Token *name) {
  LVar *lv = find_lvar(name);
  if (lv) {
    return lvar(lv);
  }

  Type *gv = find_gvar(name);
  if (gv) {
    return gvar(name, gv);
  }
  error_at(token->str, "unknown identifier");
  return NULL; // not reachable
}

Node *str() {
  Node *node = new_node(ND_STR, new_type(ARRAY));
  node->type->array_size = token->len;
  node->type->ptr_to = new_type(CHAR);
  node->index = create_str(token->str, token->len);

  token = token->next;
  return node;
}

// "(" expr ")" | ident (("(" func_call) | var)? | str | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *ident = consume_ident();
  if (ident) {
    if (consume("(")) { // function call
      return func_call(ident);
    }
    return var(ident);
  }

  if (token->kind == TK_STR) {
    return str();
  }

  return new_node_num(expect_number());
}

// "sizeof" unary | ("*" | "&") unary | ("+" | "-")? primary
Node *unary() {
  if (consume("sizeof")) {
    Node *child = unary();
    return new_node_num(size_of(child->type));
  }

  if (consume("*")) {
    Node *lhs = unary();
    Node *node = new_node(ND_DEREF, lhs->type->ptr_to);
    node->lhs = lhs;
    return node;
  }
  if (consume("&")) {
    Node *node = new_node(ND_ADDR, new_type(PTR));
    node->lhs = unary();
    node->type->ptr_to = node->lhs->type;
    return node;
  }

  if (consume("+")) {
    return primary();
  }
  if (consume("-")) {
    return new_node_child(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}

// unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node_child(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node_child(ND_DIV, node, unary());
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
      node = new_node_child(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node_child(ND_SUB, node, mul());
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
      node = new_node_child(ND_LE, node, add());
    } else if (consume("<")) {
      node = new_node_child(ND_LT, node, add());
    } else if (consume(">=")) {
      node = new_node_child(ND_LE, add(), node);
    } else if (consume(">")) {
      node = new_node_child(ND_LT, add(), node);
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
      node = new_node_child(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node_child(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node_child(ND_ASSIGN, node, assign());
  }
  return node;
}

// assign
Node *expr() { return assign(); }

// "{" stmt* "}"
Node *block() {
  expect("{");

  Node *node = new_node(ND_BLOCK, NULL);

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

// "return" expr ";"
Node *ret() {
  expect("return");

  Node *node = new_node(ND_RETURN, NULL);
  node->lhs = expr();
  expect(";");
  return node;
}

// "if" "(" expr ")" stmt ("else" stmt)?
Node *ifn() {
  expect("if");

  Node *node = new_node(ND_IF, NULL);
  expect("(");
  node->cond = expr();
  expect(")");
  node->then = stmt();

  if (consume("else")) {
    node->els = stmt();
  }
  return node;
}

// "while" "(" expr ")" stmt
Node *whilen() {
  expect("while");

  Node *node = new_node(ND_WHILE, NULL);
  expect("(");
  node->cond = expr();
  expect(")");
  node->then = stmt();
  return node;
}

// "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *forn() {
  expect("for");

  Node *node = new_node(ND_FOR, NULL);
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

  return node;
}

// ret | ifn | whilen | forn | block | var_decla | expr ";"
Node *stmt() {
  if (is_next("return")) {
    return ret();
  } else if (is_next("if")) {
    return ifn();
  } else if (is_next("while")) {
    return whilen();
  } else if (is_next("for")) {
    return forn();
  } else if (is_next("{")) {
    return block();
  } else if (is_type(token)) { // declaration
    return var_decla();
  }

  Node *node = expr();
  expect(";");
  return node;
}

// "(" (")" | type ("," expr)* ")") block
Node *func(Token *name) {
  locals = NULL;

  expect("(");

  Node *node = new_node(ND_FUNC, NULL);
  node->func = name->str;
  node->func_len = name->len;

  Node *head = NULL;
  while (!consume(")")) {
    Token *param;
    Type *type = type_ident(&param);

    Node *var = create_var(param, type);
    if (head == NULL) {
      head = var;
      node->params = head;
    } else {
      head->next = var;
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

// ("[" num "]") | ";"
Node *global_var(Token *name, Type *type) {
  Type *arr_type;
  if ((arr_type = consume_array_decla(type))) {
    type = arr_type;
  }

  Node *node = new_node(ND_GVAR_DECLA, type);
  node->name = name->str;
  node->name_len = name->len;

  expect(";");

  return node;
}

// (type_ident ("(" func | gloval_var))*
void program() {
  Token *ident;
  for (int i = 0; !at_eof(); i++) {
    Type *type = type_ident(&ident);

    if (is_next("(")) { // function
      globals[i] = func(ident);
    } else { // global var
      globals[i] = global_var(ident, type);
    }
  }
}
