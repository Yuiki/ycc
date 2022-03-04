#include <stddef.h>

typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_NUM,    // number
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // =
  ND_LVAR,   // local variable
  ND_GVAR,   // global variable
  ND_RETURN, // return
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,      // {}
  ND_CALL,       // function call
  ND_FUNC,       // function declaration
  ND_ADDR,       // &
  ND_DEREF,      // *
  ND_NOP,        // no-op
  ND_GVAR_DECLA, // global variable declaration
  ND_STR,        // string
} NodeKind;

typedef struct Type Type;

typedef enum { CHAR, INT, PTR, ARRAY } TypeKind;

struct Type {
  TypeKind ty;
  struct Type *ptr_to; // pointer if ty = PTR
  size_t array_size;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next; // next var or NULL
  char *name;
  int len;    // len of name
  int offset; // offset from RBP
  Type *type; // use if kind = ND_LVAR
};

typedef struct Str Str;

struct Str {
  Str *next; // next str or NULL
  char *value;
  int len; // len of value
  int index;
};

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *next; // next node if kind = ND_BLOCK, ND_CALL

  Node *lhs; // left side
  Node *rhs; // right side

  Node *init; // init expr if kind = ND_FOR
  Node *cond; // cond expr if kind = ND_IF, ND_WHILE, ND_FOR
  Node *step; // step expr if kind = ND_FOR
  Node *then; // then stmt if kind = ND_IF, ND_WHILE
  Node *els;  // else stmt if kind = ND_IF

  Node *body; // if kind = ND_BLOCK

  int val;    // the number if kind = ND_NUM
  int offset; // use if kind = ND_LVAR

  char *func;   // function name if kind = ND_CALL, ND_FUNC
  int func_len; // func name len if kind = ND_CALL, ND_FUNC

  Node *args; // if kind = ND_CALL

  Node *block;  // if kind = ND_FUNC
  Node *params; // if kind = ND_FUNC

  Type *type; // if kind = ND_LVAR

  LVar *locals; // if kind = ND_FUNC

  char *name;   // var name if kind = ND_GVAR, ND_GVAR_DECRA
  int name_len; // var name len if kind = ND_GVAR, ND_GVAR_DECRA

  int index; // if kind = ND_STR
};

typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT,    // identifier
  TK_NUM,
  TK_EOF,
  TK_RETURN, // return
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  TK_INT,     // int
  TK_CHAR,    // char
  TK_SIZEOF,  // sizeof
  TK_STR_LIT, // string literal
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;   // the number if kind = TK_NUM
  char *str; // token string
  int len;   // len of token
};

extern Str *strs;

// current token
extern Token *token;

// input program
extern char *user_input;

// TODO: support >100
extern Node *globals[100];

extern char *filename;

Token *tokenize(char *p);

void program();

void gen(Node *node);

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);

char *read_file(char *path);

int calc_offset(Type *type);
