#include "0cc.h"

int label_unique;

void gen_lval(Node *);
void gen_expr(Node *);
void gen_stmt(Node *);

// ベースポインタ(RBP)からのオフセットを利用して、ローカル変数のアドレスをスタックトップに置く
void gen_lval(Node *node) {
  if (node->kind != ND_DEREF && node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  if (node->kind == ND_DEREF) {
    gen_expr(node->lhs);
    return;
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_expr(Node *node) {
  if (node == NULL) {
    error("あるべき場所に式がありません\n");
    return;
  }

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
      gen_expr(node->rhs);
      printf("  pop rdi\n");  // 右辺値
      printf("  pop rax\n");  // 左辺値（変数のアドレス）
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n"); // （代入も式で、右辺値をさらに返すことに注意）
      return;
    case ND_CALL:
      for (int i = 0; i < node->args->len; i++) {
        gen_expr(node->args->ptr[i]);
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
    case ND_DEREF:
      gen_expr(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

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

void gen_stmt(Node *node) {
  if (node == NULL) {
    return;
  }

  int label = label_unique++;
  switch (node->kind) {
    case ND_BLOCK:
      for (int i = 0; i < node->stmts->len; i++) {
        gen_stmt(node->stmts->ptr[i]);
      }
      return;
    case ND_IF:
      printf(".L_IF_%d:\n", label);
      gen_expr(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if (node->els == NULL) {
        printf("  je .L_FI_%d\n", label);
        gen_stmt(node->then);
      } else {
        printf("  je .L_ELSE_%d\n", label);
        gen_stmt(node->then);
        printf("  jmp .L_FI_%d\n", label);
        printf(".L_ELSE_%d:\n", label);
        gen_stmt(node->els);
      }
      printf(".L_FI_%d:\n", label);
      return;
    case ND_WHILE:
      printf(".L_WHILE_%d:\n", label);
      gen_expr(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L_ELIHW_%d\n", label);
      gen_stmt(node->body);
      printf("  jmp .L_WHILE_%d\n", label);
      printf(".L_ELIHW_%d:\n", label);
      return;
    case ND_FOR:
      gen_expr(node->init);
      printf("  pop rax\n"); // 初期化式の返り値は利用しない
      printf(".L_FOR_%d:\n", label);
      gen_expr(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L_ROF_%d\n", label);
      gen_stmt(node->body);
      gen_expr(node->inc);
      printf("  pop rax\n"); // 更新式の返り値は利用しない
      printf("  jmp .L_FOR_%d\n", label);
      printf(".L_ROF_%d:\n", label);
      return;
    case ND_RETURN:
      gen_expr(node->lhs);
      // エピローグ
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
  }
  gen_expr(node);
  printf("  pop rax\n"); // 文としての式は返り値を利用しない
}

void gen(Node *prog) {
  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  for (int i = 0; i < prog->funcs->len; i++) {
    Node *func = prog->funcs->ptr[i];
    switch (func->kind) {
      case ND_FUNC:
        printf("%s:\n", func->name);
        // プロローグ
        printf("  push rbp\n");     // 関数呼び出し前のベースポインタをスタックに積む
        printf("  mov rbp, rsp\n"); // ベースポインタを更新
        if (func->params->len >= 1) {
          printf("  push rdi\n");
        }
        if (func->params->len >= 2) {
          printf("  push rsi\n");
        }
        if (func->params->len >= 3) {
          printf("  push rdx\n");
        }
        if (func->params->len >= 4) {
          printf("  push rcx\n");
        }
        if (func->params->len >= 5) {
          printf("  push r8\n");
        }
        if (func->params->len >= 6) {
          printf("  push r9\n");
        }
        printf("  sub rsp, %d\n", 8 * (func->lvars->len - func->params->len));
        gen_stmt(func->body);
        continue;
    }
  }
}
