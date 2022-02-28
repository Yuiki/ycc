#include "ycc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *user_input;

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

// tokenize and return true if `kw` is keyword
// otherwise, return false
bool tokenize_kw(char **p, Token **cur, char *kw, TokenKind kind) {
  int len = strlen(kw);
  if (!strncmp(*p, kw, len) && !is_alnum((*p)[len])) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = *p;
    tok->len = len;
    *p += len;

    (*cur)->next = tok;
    (*cur) = tok;
    return true;
  }
  return false;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) ||
        !strncmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      p += 2;
      cur->len = 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' ||
        *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if ((tokenize_kw(&p, &cur, "return", TK_RETURN))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "if", TK_IF))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "else", TK_ELSE))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "while", TK_WHILE))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "for", TK_FOR))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "int", TK_INT))) {
      continue;
    }

    if ((tokenize_kw(&p, &cur, "sizeof", TK_SIZEOF))) {
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    char *start = p;
    while (is_alnum(*p)) {
      p++;
    }
    if (start != p) { // found identifier
      cur = new_token(TK_IDENT, cur, start);
      cur->len = p - start;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
