#include "0cc.h"

// ベースポインタ(RBP)からのオフセットを利用して、ローカル変数のアドレスをスタックトップに置く
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      // スタックトップにローカル変数のアドレスが入っているので、メモリからそのアドレスの値をロードしてスタックトップに置く
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");  // 右辺値
      printf("  pop rax\n");  // 左辺値（変数のアドレス）
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n"); // （代入も式で、右辺値をさらに返すことに注意）
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");      // RAXを引き延ばしてRDXとRAXにセット
      printf("  idiv rdi\n"); // RDXとRAXを128ビットとみなしてRDIで割る
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");     // フラグレジスタの値をAL(= RAXの下位8bit)にセット
      printf("  movzb rax, al\n"); // 上位56bitをゼロ埋めする
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
