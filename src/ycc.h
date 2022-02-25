
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // number
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs; // left side
  Node *rhs; // right side
  int val;   // the number if kind = ND_NUM
};

typedef enum {
  TK_RESERVED, // symbol
  TK_NUM,
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

// current token
extern Token *token;

// input program
extern char *user_input;

Token *tokenize(char *p);

Node *expr();

void gen(Node *node);

void error_at(char *loc, char *fmt, ...);
