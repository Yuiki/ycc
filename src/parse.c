#include "ycc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LVar LVar;

struct LVar {
  LVar *next; // next var or NULL
  char *name;
  int len;    // len of name
  int offset; // offset from RBP
};

// local vars
LVar *locals;

Token *token;

Node *functions[100];

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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
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

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Node *expr();

Node *create_var(Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

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

  return node;
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
      return create_var(tok);
    }
  }

  return new_node_num(expect_number());
}

Node *unary() {
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
  } else {
    node = expr();
    expect(";");
  }

  return node;
}

Node *function() {
  if (token->kind != TK_IDENT) {
    error_at(token->str, "関数名ではありません");
  }
  Token *tok = token;
  token = token->next;

  expect("(");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->func = tok->str;
  node->func_len = tok->len;

  Node *head = NULL;
  while (!consume(")")) {
    Token *param = consume_ident();
    if (head == NULL) {
      head = create_var(param);
      node->params = head;
    } else {
      head->next = create_var(param);
      head = head->next;
    }
    if (!consume(",")) {
      expect(")");
      break;
    }
  }

  node->block = block();

  return node;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    functions[i++] = function();
  }
  functions[i] = NULL;
}
