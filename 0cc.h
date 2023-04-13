#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char*, ...);
void error_at(char*, char*, ...);

typedef enum {
  TK_RESERVED, // 記号
  TK_RETURN,   // return
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

Token *tokenize(char*);

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
  ND_RETURN, // return
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ使う
  int offset;    // kindがND_LVARの場合のみ使う
};

Node **parse(Token*);

void gen(Node*);
