#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
  TK_NUM = 256,
  TK_EOF,
};

// トークン型
typedef struct {
  int ty;
  int val;
  char *input;
} Token;

// 入力プログラム
char *user_input;

// トークナイズした結果のトークン列 < 100
Token tokens[100];

// エラーを報告する関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する関数
void error_at(char *loc, char *msg) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

// user_inputをトークンに分割してtokensに保存する
void tokenize() {
  char *p = user_input;

  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の数エラー");
    return 1;
  }

  // トークナイズする
  user_input = argv[1];
  tokenize();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 必要な式の最初の数を確かめて最初のmov命令
  if (tokens[0].ty != TK_NUM) {
    error_at(tokens[0].input, "数ではありません");
  }
  printf("  mov rax, %d\n", tokens[0].val);

  // "+ 数" もしくは "- 数" というトークンの並びを消費しつつアセンブリを出力
  int i = 1;
  while (tokens[i].ty != TK_EOF) {
    if (tokens[i].ty == '+') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        error_at(tokens[i].input, "数ではありません");
      }
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if (tokens[i].ty == '-') {
      i++;
      if (tokens[i].ty != TK_NUM) {
        error_at(tokens[i].input, "数ではありません");
      }
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    error_at(tokens[i].input, "予期しないトークンです");
  }

  printf("  ret\n");
  return 0;
}
