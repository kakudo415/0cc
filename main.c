#include "0cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  Token *token = tokenize(argv[1]);
  Node *prog = parse(token);
  gen(prog);

  return 0;
}

