#include "0cc.h"

int label_unique;

// ベースポインタ(RBP)からのオフセットを利用して、ローカル変数のアドレスをスタックトップに置く
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node == NULL) {
    printf("  push 0\n"); // 空文も後でpopされてしまうので、対症療法
    return;
  }
  int label;
  switch (node->kind) {
    case ND_FUNC:
      printf("%s:\n", node->name);
      // プロローグ
      printf("  push rbp\n");       // 関数呼び出し前のベースポインタをスタックに積む
      printf("  mov rbp, rsp\n");   // ベースポインタを更新
      if (node->params->len >= 1) {
        printf("  push rdi\n");
      }
      if (node->params->len >= 2) {
        printf("  push rsi\n");
      }
      if (node->params->len >= 3) {
        printf("  push rdx\n");
      }
      if (node->params->len >= 4) {
        printf("  push rcx\n");
      }
      if (node->params->len >= 5) {
        printf("  push r8\n");
      }
      if (node->params->len >= 6) {
        printf("  push r9\n");
      }
      printf("  sub rsp, %d\n", 8 * (node->lvars->len - node->params->len));
      gen(node->body);
      // エピローグ
      printf("  mov rsp, rbp\n");   // スタックを崩す
      printf("  pop rbp\n");        // 積んでおいた関数呼び出し前のベースポインタを思い出す
      printf("  ret\n");
      return;
    case ND_BLOCK:
      for (int i = 0; i < node->stmts->len; i++) {
        gen(node->stmts->ptr[i]);
        printf("  pop rax\n");
      }
      return;
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
      return;
    case ND_CALL:
      for (int i = 0; i < node->args->len; i++) {
        gen(node->args->ptr[i]);
      }
      if (node->args->len >= 6) {
        printf("  pop r9\n"); // 第6引数
      }
      if (node->args->len >= 5) {
        printf("  pop r8\n"); // 第5引数
      }
      if (node->args->len >= 4) {
        printf("  pop rcx\n"); // 第4引数
      }
      if (node->args->len >= 3) {
        printf("  pop rdx\n"); // 第3引数
      }
      if (node->args->len >= 2) {
        printf("  pop rsi\n"); // 第2引数
      }
      if (node->args->len >= 1) {
        printf("  pop rdi\n"); // 第1引数
      }
      printf("  call %s\n", node->name); // 「%*s」とすると「%数字s」の数字部分を変数で渡せる
      printf("  push rax\n");            // 返り値をスタックに積む
      // TODO: 引数が、レジスタを超えてスタックに渡るときは、RSPの16バイトアライメントに注意！
      return;
    case ND_IF:
      label = label_unique++;
      printf(".L_IF_%d:\n", label);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if (node->els == NULL) {
        printf("  je .L_FI_%d\n", label);
        gen(node->then);
      } else {
        printf("  je .L_ELSE_%d\n", label);
        gen(node->then);
        printf("  jmp .L_FI_%d\n", label);
        printf(".L_ELSE_%d:\n", label);
        gen(node->els);
      }
      printf(".L_FI_%d:\n", label);
      printf("  push rax\n"); // 文もあとでpopされてしまう問題の対症療法
      return;
    case ND_WHILE:
      label = label_unique++;
      printf(".L_WHILE_%d:\n", label);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L_ELIHW_%d\n", label);
      gen(node->body);
      printf("  jmp .L_WHILE_%d\n", label);
      printf(".L_ELIHW_%d:\n", label);
      printf("  push rax\n"); // 文もあとでpopされてしまう問題の対症療法
      return;
    case ND_FOR:
      label = label_unique++;
      gen(node->init);
      printf(".L_FOR_%d:\n", label);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L_ROF_%d\n", label);
      gen(node->body);
      gen(node->inc);
      printf("  jmp .L_FOR_%d\n", label);
      printf(".L_ROF_%d:\n", label);
      printf("  push rax\n"); // 文もあとでpopされてしまう問題の対症療法
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_DEREF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
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
