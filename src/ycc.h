#include <stdbool.h>
#include <stddef.h>

typedef enum {
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_MOD,        // %
  ND_NUM,        // number
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_LAND,       // &&
  ND_LOR,        // ||
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
  ND_BREAK,      // break
  ND_CONTINUE,   // continue
  ND_IF,         // if
  ND_SWITCH,     // switch
  ND_CASE,       // case
  ND_DEFAULT,    // case
  ND_WHILE,      // while
  ND_FOR,        // for
  ND_BLOCK,      // {}
  ND_NOP,        // no-op
} NodeKind;

typedef enum { CHAR, INT, VOID, PTR, ARRAY, ENUM, STRUCT } TypeKind;

typedef struct Type Type;

typedef struct StructMember StructMember;

struct Type {
  TypeKind kind;
  Type *ptr_to; // pointer if ty = PTR
  size_t array_size;
  char *name;           // if ty = ENUM, STRUCT
  int name_len;         // if ty = ENUM, STRUCT
  bool needs_specifier; // if ty = STRUCT
  StructMember *member; // if ty = STRUCT
  Type *next;           // if parent is Scope
};

struct StructMember {
  Type *type;
  char *name;
  int name_len;
  int offset;
  StructMember *next;
};

typedef enum { GVAR, LVAR, ENUM_CONST } IdentKind;

typedef struct Ident Ident;

struct Ident {
  Ident *next; // next var or NULL
  char *name;
  int len; // len of name
  IdentKind kind;
  int offset; // offset from RBP if kind = VAR
  Type *type; // if kind = VAR
  int value;  // if kind = ENUM_CONST
};

typedef struct Scope Scope;

struct Scope {
  Ident *ident; // head
  Type *type;   // head
  Scope *parent;
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
  Node *cond; // cond expr if kind = ND_IF, ND_WHILE, ND_FOR, ND_SWITCH
  Node *step; // step expr if kind = ND_FOR
  Node *then; // then stmt if kind = ND_IF, ND_WHILE, ND_SWITCH
  Node *els;  // else stmt if kind = ND_IF

  Node *body; // if kind = ND_BLOCK

  int val; // the number if kind = ND_NUM, ND_CASE

  int offset; // if kind = ND_LVAR

  char *func;   // function name if kind = ND_CALL, ND_FUNC
  int func_len; // func name len if kind = ND_CALL, ND_FUNC

  Node *args; // if kind = ND_CALL

  Node *block;  // if kind = ND_FUNC
  Node *params; // if kind = ND_FUNC

  Type *type;

  int lvars_size; // if kind = ND_FUNC

  char *name;   // var name if kind = ND_GVAR, ND_GVAR_DECLA
  int name_len; // var name len if kind = ND_GVAR, ND_GVAR_DECLA

  int g_index; // if kind = ND_STR
  int label;   // if kind = ND_CASE
};

typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT,    // identifier
  TK_NUM,
  TK_STR,  // string literal
  TK_CHAR, // char constant
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

int offset_of(int curr_size, Type *type);
