#include "ycc.h"
#include <ctype.h>
#include <stdarg.h>
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
        *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (!strncmp(p, "return", 6) && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p);
      p += 6;
      cur->len = 6;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    char *start = p;
    while ('a' <= *p && *p <= 'z') {
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
