#include <stddef.h>

typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_NUM,        // number
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_GVAR_DECLA, // global variable declaration
  ND_GVAR,       // global variable
  ND_LVAR,       // local variable
  ND_ADDR,       // &
  ND_DEREF,      // *
  ND_CALL,       // function call
  ND_FUNC,       // function declaration
  ND_STR,        // string
  ND_ASSIGN,     // =
  ND_RETURN,     // return
  ND_IF,         // if
  ND_WHILE,      // while
  ND_FOR,        // for
  ND_BLOCK,      // {}
  ND_NOP,        // no-op
} NodeKind;

typedef enum { CHAR, INT, PTR, ARRAY } TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  struct Type *ptr_to; // pointer if ty = PTR
  size_t array_size;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next; // next var or NULL
  char *name;
  int len;    // len of name
  int offset; // offset from RBP
  Type *type;
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

  int val; // the number if kind = ND_NUM

  int offset; // if kind = ND_LVAR

  char *func;   // function name if kind = ND_CALL, ND_FUNC
  int func_len; // func name len if kind = ND_CALL, ND_FUNC

  Node *args; // if kind = ND_CALL

  Node *block;  // if kind = ND_FUNC
  Node *params; // if kind = ND_FUNC

  Type *type;

  LVar *locals; // if kind = ND_FUNC

  char *name;   // var name if kind = ND_GVAR, ND_GVAR_DECLA
  int name_len; // var name len if kind = ND_GVAR, ND_GVAR_DECLA

  int index; // if kind = ND_STR
};

typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT,    // identifier
  TK_NUM,
  TK_STR, // string literal
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;   // the number if kind = TK_NUM
  char *str; // token string
  int len;   // len of token
};

extern char *filename;

// input program
extern char *user_input;

// current token
extern Token *token;

extern Str *strs;

// TODO: support >100
extern Node *globals[100];

// tokenize.c

Token *tokenize(char *p);

// parse.c
void program();

// codegen.c

void gen_program();

// error.c

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);

// file.c

char *read_file(char *path);

// size.c

int size_of(Type *type);

int calc_stack_size(LVar *locals);
