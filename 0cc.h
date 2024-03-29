#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char*, ...);
void error_at(char*, char*, ...);

typedef struct Vec {
  void **ptr;
  int cap;
  int len;
} Vec;

void *new_vec();
void vec_push(Vec*, void*);
void *vec_pop(Vec*);

typedef enum {
  TK_RESERVED, // 記号
  TK_IF,       // if
  TK_ELSE,     // else
  TK_WHILE,    // while
  TK_FOR,      // for
  TK_RETURN,   // return
  TK_SIZEOF,   // sizeof
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数
  TK_EOF,      // EOF
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind; // 型
  Token *next;    // 次
  int val;        // kindがTK_NUMのとき、その値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

bool startswith(char *, char *);
Token *tokenize(char*);

typedef enum {
  TY_INT,
  TY_PTR,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind typ;
  struct Type *ptr_to;
};

typedef enum {
  ND_PROGRAM,
  ND_FUNC,    // 関数定義
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_DEREF, // *
  ND_ADDR,  // &
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
  ND_CALL,   // 関数呼び出し
  ND_BLOCK,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_RETURN, // return
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型

  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
  Type *typ;

  // CALL: name(arg, arg, ...)
  // DEF:  name(param, param, ...)
  char *name;
  Vec *args;
  Vec *params;
  Vec *lvars;

  // "if" (cond) then "else" els
  // "while" (cond) body
  // "for" (init; cond; inc) body
  Node *cond;
  Node *then;
  Node *els;
  Node *init;
  Node *body;
  Node *inc;

  // Compound Statement
  Vec *stmts;

  // Top Level Declarations
  Vec *funcs;
};

typedef struct Var Var;
struct Var {
  char *name;
  int offset;
  Type *typ;
};

Node *parse(Token*);

void gen(Node*);
