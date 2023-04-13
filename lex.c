#include "0cc.h"

// 入力プログラム
extern char *user_input;
char *user_input;

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

// 文字列pが、文字列qで始まっているか？
bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0; // strlen()はNULL文字の直前までの文字数
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  user_input = p;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p++, 1); // pの指す演算子をトークン化したあと、pをインクリメント
      continue;
    }

    // ラテン文字1文字の変数
    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;                  // 整数リテラルの先頭を覚えておいて、後で桁数を数える
      cur->val = strtol(p, &p, 10); // strtol()はlong型に変換できなくなった文字を&pに入れてくれる。つまり、pが数値の終わりまで勝手にインクリメントされる！
      cur->len = p - q;
      continue;
    }
    
    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
