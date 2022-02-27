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
  ND_RETURN, // return
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK, // {}
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;

  Node *lhs; // left side
  Node *rhs; // right side

  Node *init; // init expr if kind = ND_FOR
  Node *cond; // cond expr if kind = ND_IF, ND_WHILE, ND_FOR
  Node *step; // step expr if kind = ND_FOR
  Node *then; // then stmt if kind = ND_IF, ND_WHILE
  Node *els;  // else stmt if kind = ND_IF

  Node *body; // if kind = ND_BLOCK
  Node *next; // next stmt if body

  int val;    // the number if kind = ND_NUM
  int offset; // use if kind = ND_LVAR
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
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;   // the number if kind = TK_NUM
  char *str; // token string
  int len;   // len of token
};

// current token
extern Token *token;

// input program
extern char *user_input;

extern Node *code[100];

Token *tokenize(char *p);

void program();

void gen(Node *node);

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);
