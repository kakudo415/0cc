#include "0cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  Token *token = tokenize(argv[1]);
  Node *program = parse(token);

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  // 先頭の式から順にコード生成
  for (int i = 0; i < program->funcs->len; i++) {
    gen(program->funcs->ptr[i]);
  }

  return 0;
}

