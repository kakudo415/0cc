#include "0cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  Token *token = tokenize(argv[1]);
  Node **program = parse(token);

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");     // 関数呼び出し前のベースポインタをスタックに積む
  printf("  mov rbp, rsp\n"); // ベースポインタを更新
  printf("  sub rsp, 208\n"); // 変数個分スタックを伸ばす

  // 先頭の式から順にコード生成
  for (int i = 0; program[i]; i++) {
    gen(program[i]);
    // 式の評価結果がスタックに積まれているので、毎回取り出しておく
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているので、それを返り値とする
  printf("  mov rsp, rbp\n"); // スタックを崩す
  printf("  pop rbp\n");      // 積んでおいた関数呼び出し前のベースポインタを思い出す
  printf("  ret\n");
  return 0;
}

