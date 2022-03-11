#include "ycc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scope *scope;

int lvars_size;

Str *strs;

Node *globals[100];

Node *expr();

Node *stmt();

Node *equality();

int case_label;

Node *new_node(NodeKind kind, Type *type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->type = type;
  return node;
}

Type *new_type(TypeKind ty) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = ty;
  return type;
}

Ident *new_ident(Type *type, char *name, int name_len, IdentKind kind) {
  Ident *ident = calloc(1, sizeof(Ident));
  ident->name = name;
  ident->len = name_len;
  ident->kind = kind;
  ident->type = type;
  return ident;
}

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

// "enum" "{" ident ("," ident)* ","? "}"
void enum_specifier() {
  expect("enum");
  expect("{");
  for (int i = 0;; i++) {
    Token *tok = consume_ident();
    if (tok == NULL) {
      error_at(token->str, "not identifier");
    }

    Ident *ident = new_ident(new_type(INT), tok->str, tok->len, ENUM_CONST);
    ident->value = i;
    ident->next = scope->ident;
    scope->ident = ident;

    if (!consume(",")) {
      break;
    }
  }
  consume(",");
  expect("}");
}

Type *consume_type(Token *token) {
  if (consume("char")) {
    return new_type(CHAR);
  }
  if (consume("int")) {
    return new_type(INT);
  }
  if (consume("void")) {
    return new_type(VOID);
  }
  if (is_next("enum")) {
    enum_specifier();
    return new_type(ENUM);
  }
  return NULL;
}

Type *expect_type_specifier() {
  Type *type = consume_type(token);
  if (type == NULL) {
    error_at(token->str, "unknown type");
  }

  while (consume("*")) {
    Type *new_ty = new_type(PTR);
    new_ty->ptr_to = type;
    type = new_ty;
  }

  return type;
}

bool is_next_decla() {
  return is_next("char") || is_next("int") || is_next("void") ||
         is_next("enum");
}

bool at_eof() { return token->kind == TK_EOF; }

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

Ident *find_var(Token *tok, bool current) {
  for (Scope *sc = scope; sc; sc = sc->parent) {
    for (Ident *var = sc->ident; var; var = var->next) {
      if (var && var->len == tok->len &&
          !memcmp(tok->str, var->name, var->len)) {
        return var;
      }
    }
    if (current) {
      break;
    }
  }
  return NULL;
}

Node *create_var(Token *tok, Type *type) {
  Node *node = new_node(ND_LVAR, type);

  Ident *lvar = find_var(tok, true);
  if (lvar) {
    error_at(token->str, "変数が定義済みです");
  }

  lvar = new_ident(type, tok->str, tok->len, LVAR);
  lvar->next = scope->ident;
  scope->ident = lvar;

  int diff = size_of(type);
  lvars_size += diff;
  lvar->offset = lvars_size;
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
  Type *type = expect_type_specifier();

  *ident = consume_ident();
  return type;
}

// "[" num? "]""
Type *consume_array_decla(Type *base_ty, int *len) {
  if (!consume("[")) {
    return NULL;
  }
  Type *new_ty = new_type(ARRAY);

  if (token->kind == TK_NUM) {
    *len = token->val;
    token = token->next;

    new_ty->array_size = *len;
  } else {
    *len = -1;
  }

  expect("]");

  new_ty->ptr_to = base_ty;
  return new_ty;
}

Node *create_arr_elem(Node *arr, Node *offset) {
  Node *add = new_node_child(ND_ADD, arr, offset);
  Node *deref = new_node(ND_DEREF, add->type->ptr_to);
  deref->lhs = add;
  return deref;
}

// type-ident (array-decla)?
// ("=" (equality | ("{" equality? ("," equality)* "}")))? ";"
Node *decla() {
  Token *ident;
  Type *type = type_ident(&ident);
  if (ident == NULL) {
    expect(";");
    return new_node(ND_NOP, NULL);
  }

  Type *arr_type;
  int arr_len;
  if ((arr_type = consume_array_decla(type, &arr_len))) {
    type = arr_type;
  }

  Node *var = create_var(ident, type);

  if (consume("=")) {
    if (consume("{")) {
      Node *node = new_node(ND_BLOCK, NULL);

      Node *head = NULL;
      int init_idx;
      for (init_idx = 0;; init_idx++) {
        Node *offset = new_node_num(init_idx);
        Node *elem = create_arr_elem(var, offset);

        Node *next = new_node_child(ND_ASSIGN, elem, equality());
        if (head == NULL) {
          head = next;
          node->body = head;
        } else {
          head->next = next;
          head = head->next;
        }

        if (consume(",")) {
          continue;
        } else {
          expect("}");
          break;
        }
      }

      if (arr_len == -1) {
        var->type->array_size = init_idx + 1;
        var->offset += size_of(var->type);
      }

      // zero initialization
      Node *zero = new_node_num(0);
      for (int i = init_idx + 1; i < arr_len; i++) {
        Node *offset = new_node_num(i);
        Node *elem = create_arr_elem(var, offset);

        Node *next = new_node_child(ND_ASSIGN, elem, zero);
        if (head == NULL) {
          head = next;
          node->body = head;
        } else {
          head->next = next;
          head = head->next;
        }
      }

      expect(";");
      return node;
    }

    if (arr_len == -1) {
      error_at(token->str, "unknown array size");
    }

    Node *node = new_node_child(ND_ASSIGN, var, equality());
    expect(";");
    return node;
  } else {
    expect(";");
    return new_node(ND_NOP, NULL);
  }
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
    Node *elem = create_arr_elem(base, expr());
    expect("]");
    return elem;
  }
  return NULL;
}

Node *lvar(Type *type, int offset) {
  Node *node = new_node(ND_LVAR, type);
  node->offset = offset;

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
  Ident *var = find_var(name, false);
  if (!var) {
    error_at(name->str, "unknown identifier");
  }

  if (var->kind == LVAR) {
    return lvar(var->type, var->offset);
  }

  if (var->kind == GVAR) {
    return gvar(name, var->type);
  }

  if (var->kind == ENUM_CONST) {
    return new_node_num(var->value);
  }

  error_at(name->str, "unknown var kind");
  return NULL; // not reachable
}

Node *str() {
  Node *node = new_node(ND_STR, new_type(ARRAY));
  node->type->array_size = token->len;
  node->type->ptr_to = new_type(CHAR);
  node->g_index = create_str(token->str, token->len);

  token = token->next;
  return node;
}

Node *character() {
  Node *node = new_node(ND_NUM, new_type(CHAR));
  node->val = token->val;
  token = token->next;
  return node;
}

// character | integer
Node *constant() {
  if (token->kind == TK_CHAR) {
    return character();
  }

  return new_node_num(expect_number());
}

// "(" expr ")" | ident (("(" func_call) | var )? | str | constant
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

  return constant();
}

// primary ("++" | "--")*
Node *postfix() {
  Node *node = primary();
  for (;;) {
    if (consume("++")) {
      Node *add = new_node_child(ND_ADD, node, new_node_num(1));
      Node *assign = new_node_child(ND_ASSIGN, node, add);
      Node *sub = new_node_child(ND_SUB, assign, new_node_num(1));
      node = sub;
      continue;
    }
    if (consume("--")) {
      Node *sub = new_node_child(ND_SUB, node, new_node_num(1));
      Node *assign = new_node_child(ND_ASSIGN, node, sub);
      Node *add = new_node_child(ND_ADD, assign, new_node_num(1));
      node = add;
      continue;
    }
    return node;
  }
}

// ("sizeof" | "&" | "*" | "+" | "-" | "!") unary | postfix
Node *unary() {
  if (consume("sizeof")) {
    Node *child = unary();
    return new_node_num(size_of(child->type));
  }

  if (consume("&")) {
    Node *node = new_node(ND_ADDR, new_type(PTR));
    node->lhs = unary();
    node->type->ptr_to = node->lhs->type;
    return node;
  }

  if (consume("*")) {
    Node *lhs = unary();
    Node *node = new_node(ND_DEREF, lhs->type->ptr_to);
    node->lhs = lhs;
    return node;
  }

  if (consume("+")) {
    return unary();
  }

  if (consume("-")) {
    return new_node_child(ND_SUB, new_node_num(0), unary());
  }

  if (consume("!")) {
    return new_node_child(ND_EQ, unary(), new_node_num(0));
  }

  return postfix();
}

// unary (("*" | "/" | "%") unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node_child(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node_child(ND_DIV, node, unary());
    } else if (consume("%")) {
      node = new_node_child(ND_MOD, node, unary());
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

// equality ("&&" logical_and)*
Node *logical_and() {
  Node *node = equality();
  for (;;) {
    if (consume("&&")) {
      node = new_node_child(ND_LAND, node, logical_and());
    } else {
      return node;
    }
  }
}

// logical_and ("||" logical_or)*
Node *logical_or() {
  Node *node = logical_and();
  for (;;) {
    if (consume("||")) {
      node = new_node_child(ND_LOR, node, logical_or());
    } else {
      return node;
    }
  }
}

// logical_or (("=" | "+=") assign)?
Node *assign() {
  Node *node = logical_or();
  if (consume("=")) {
    node = new_node_child(ND_ASSIGN, node, assign());
  }
  if (consume("+=")) {
    Node *add = new_node_child(ND_ADD, node, assign());
    node = new_node_child(ND_ASSIGN, node, add);
  }
  return node;
}

// assign
Node *expr() { return assign(); }

void begin_scope() {
  Scope *new_scope = calloc(1, sizeof(Scope));
  new_scope->parent = scope;
  scope = new_scope;
}

void end_scope() { scope = scope->parent; }

// "case" expr ":" statement
Node *case_stmt() {
  expect("case");

  Node *node = new_node(ND_CASE, NULL);
  Node *cond = logical_or();
  // TODO: suppport compile-time calcuration
  if (cond->kind != ND_NUM) {
    error_at(token->str, "should be constant");
  }
  node->val = cond->val;
  expect(":");
  node->rhs = stmt();
  node->label = case_label++;

  return node;
}

// "default" ":" statement
Node *default_stmt() {
  expect("default");
  expect(":");

  Node *node = new_node(ND_DEFAULT, NULL);
  node->rhs = stmt();
  node->label = case_label++;

  return node;
}

// "{" (stmt | decla)* "}"
Node *compound_stmt() {
  expect("{");

  Node *node = new_node(ND_BLOCK, NULL);

  Node *head = NULL;
  while (!consume("}")) {
    Node *child;
    if (is_next_decla()) {
      child = decla();
    } else {
      child = stmt();
    }
    if (head == NULL) {
      head = child;
      node->body = head;
      continue;
    }
    head->next = child;
    head = head->next;
  }

  return node;
}

// "continue" ";"
Node *cont_stmt() {
  expect("continue");
  Node *node = new_node(ND_CONTINUE, NULL);
  expect(";");
  return node;
}

// "break" ";"
Node *break_stmt() {
  expect("break");
  Node *node = new_node(ND_BREAK, NULL);
  expect(";");
  return node;
}

// "return" expr? ";"
Node *ret_stmt() {
  expect("return");

  Node *node = new_node(ND_RETURN, NULL);
  if (!consume(";")) {
    node->lhs = expr();
    expect(";");
    return node;
  }
  return node;
}

// "if" "(" expr ")" stmt ("else" stmt)?
Node *if_stmt() {
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

// "switch" "(" expr ")" stmt
Node *switch_stmt() {
  expect("switch");

  Node *node = new_node(ND_SWITCH, NULL);
  expect("(");
  node->cond = expr();
  expect(")");
  node->then = stmt();

  return node;
}

// "while" "(" expr ")" stmt
Node *while_stmt() {
  expect("while");

  Node *node = new_node(ND_WHILE, NULL);
  expect("(");
  node->cond = expr();
  expect(")");
  node->then = stmt();
  return node;
}

// "for" "(" (decla? | expr? ";") expr? ";" expr? ")" stmt
Node *for_stmt() {
  expect("for");

  Node *node = new_node(ND_FOR, NULL);
  expect("(");

  if (is_next_decla()) { // declaration
    node->init = decla();
  } else if (!consume(";")) {
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

// case_stmt  | default_stmt | compound_stmt | if_stmt | switch_stmt |
// while_stmt | for_stmt | cont_stmt | break_stmt | ret_stmt | expr ";"
Node *stmt() {
  if (is_next("case")) {
    return case_stmt();
  }
  if (is_next("default")) {
    return default_stmt();
  }
  if (is_next("{")) {
    begin_scope();
    Node *stmts = compound_stmt();
    end_scope();
    return stmts;
  }
  if (is_next("if")) {
    return if_stmt();
  }
  if (is_next("switch")) {
    return switch_stmt();
  }
  if (is_next("while")) {
    return while_stmt();
  }
  if (is_next("for")) {
    return for_stmt();
  }
  if (is_next("continue")) {
    return cont_stmt();
  }
  if (is_next("break")) {
    return break_stmt();
  }
  if (is_next("return")) {
    return ret_stmt();
  }

  Node *node = expr();
  expect(";");
  return node;
}

// "(" (")" | type ("," expr)* ")") compound_stmt
Node *func(Token *name) {
  begin_scope();

  expect("(");

  Node *node = new_node(ND_FUNC, NULL);
  node->func = name->str;
  node->func_len = name->len;

  Node *head = NULL;
  while (!consume(")")) {
    Token *param;
    Type *type = type_ident(&param);

    if (param != NULL) {
      Node *var = create_var(param, type);
      if (head == NULL) {
        head = var;
        node->params = head;
      } else {
        head->next = var;
        head = head->next;
      }
    }

    if (!consume(",")) {
      expect(")");
      break;
    }
  }

  node->block = compound_stmt();

  node->lvars_size = lvars_size + ((16 - (lvars_size % 16)) % 16);
  lvars_size = 0;

  end_scope();

  return node;
}

// ("[" num "]") | ";"
Node *global_var(Token *name, Type *type) {
  Type *arr_type;
  int arr_len;
  if ((arr_type = consume_array_decla(type, &arr_len))) {
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
  begin_scope();
  Ident **head = &scope->ident;

  Token *ident;
  for (int i = 0; !at_eof();) {
    Type *type = type_ident(&ident);
    if (ident == NULL) {
      expect(";");
      continue;
    }

    if (is_next("(")) { // function
      Node *t = func(ident);
      globals[i] = t;
    } else { // global var
      Node *node = global_var(ident, type);
      globals[i] = node;

      Ident *ident = new_ident(node->type, node->name, node->name_len, GVAR);
      *head = ident;
      head = &ident->next;
    }
    i++;
  }
}
