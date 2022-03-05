#include "ycc.h"
#include <ctype.h>
#include <stdbool.h>
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

// is alphabet, number or _
bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

// tokenize and return true if `kw` is keyword
// otherwise, return false
bool tokenize_kw(char **pp, Token **pcur, char *kw, TokenKind kind) {
  int len = strlen(kw);
  if (!strncmp(*pp, kw, len) && !is_alnum((*pp)[len])) {
    *pcur = new_token(kind, *pcur, *pp);
    (*pcur)->len = len;
    *pp += len;
    return true;
  }
  return false;
}

bool skip(char **pp) {
  if (isspace(**pp)) {
    (*pp)++;
    return true;
  }
  return false;
}

bool tokenize_reserved(char **pp, Token **pcur) {
  char *p = *pp;

  if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) ||
      !strncmp(p, ">=", 2)) {
    *pcur = new_token(TK_RESERVED, *(pcur), p);
    (*pcur)->len = 2;
    *pp += 2;
    return true;
  }

  if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
      *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' ||
      *p == '{' || *p == '}' || *p == ',' || *p == '*' || *p == '&' ||
      *p == '[' || *p == ']') {
    *pcur = new_token(TK_RESERVED, *(pcur), (*pp)++);
    (*pcur)->len = 1;
    return true;
  }

  return false;
}

bool tokenize_keywords(char **p, Token **cur) {
  if ((tokenize_kw(p, cur, "return", TK_RETURN))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "if", TK_IF))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "else", TK_ELSE))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "while", TK_WHILE))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "for", TK_FOR))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "int", TK_INT))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "char", TK_CHAR))) {
    return true;
  }

  if ((tokenize_kw(p, cur, "sizeof", TK_SIZEOF))) {
    return true;
  }

  return false;
}

bool tokenize_num(char **pp, Token **pcur) {
  if (isdigit(**pp)) {
    *pcur = new_token(TK_NUM, *pcur, *pp);
    (*pcur)->val = strtol(*pp, pp, 10);
    return true;
  }
  return false;
}

bool tokenize_str(char **pp, Token **pcur) {
  char *p = *pp;

  if (*p == '\"') {
    *pcur = new_token(TK_STR_LIT, *pcur, p);
    (*pcur)->val = strtol(p, pp, 10);

    char *start = p;
    (*pp)++;

    while (**pp != '\"') {
      (*pp)++;
    }

    (*pp)++;
    (*pcur)->len = *pp - start;
    return true;
  }
  return false;
}

bool tokenize_ident(char **pp, Token **pcur) {
  char *start = *pp;
  while (is_alnum(**pp)) {
    (*pp)++;
  }
  if (start != *pp) { // found identifier
    *pcur = new_token(TK_IDENT, *pcur, start);
    (*pcur)->len = *pp - start;
    return true;
  }
  return false;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (skip(&p)) {
      continue;
    }

    if (tokenize_reserved(&p, &cur)) {
      continue;
    }

    if (tokenize_keywords(&p, &cur)) {
      continue;
    }

    if (tokenize_num(&p, &cur)) {
      continue;
    }

    if (tokenize_str(&p, &cur)) {
      continue;
    }

    if (tokenize_ident(&p, &cur)) {
      continue;
    }

    error_at(p, "cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}
