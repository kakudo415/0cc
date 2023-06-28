#include "0cc.h"

Node *prog;

// 入力プログラム
extern char *user_input;

// 現在注目しているトークン
Token *token;

// エラーを報告する関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input; // エラー箇所のポインタ - 入力先頭のポインタ
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待しているトークン型のときには、1つ読み進めて真を返す。
bool consume_keyword(TokenKind kind) {
  if (token->kind != kind) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが識別子の場合は、トークンを1つ読み進めて識別子トークンを返す
Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }
  Token *ident_token = token;
  token = token->next;
  return ident_token;
}

// 次のトークンが期待している記号の場合、トークンを1つ読み進める
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
    error_at(token->str, "'%s'ではありません", op);
  }
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその値を返す
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}  

bool at_eof() {
  return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  // TODO: Implement proper type inference
  Type *typ = calloc(1, sizeof(Type));
  typ->typ = TY_INT;
  if (node->lhs != NULL) {
    if (node->lhs->typ->typ == TY_PTR) {
      typ->typ = TY_PTR;
    }
  }
  if (node->rhs != NULL) {
    if (node->rhs->typ->typ == TY_PTR) {
      typ->typ = TY_PTR;
    }
  }
  node->typ = typ;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  Type *typ = calloc(1, sizeof(Type));
  typ->typ = TY_INT;
  node->typ = typ;
  return node;
}

void global();
Var *param();
Type *typ();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Vec *lvars;

// ローカル変数を名前で検索する
Var *find_lvar(Token *tok) {
  for (int i = 0; i < lvars->len; i++) {
    Var *lvar = lvars->ptr[i];
    if (strlen(lvar->name) == tok->len && !memcmp(lvar->name, tok->str, tok->len)) {
      return lvar;
    }
  }
  return NULL;
}

Node *parse(Token *head) {
  token = head;
  prog = calloc(1, sizeof(Node));
  prog->funcs = new_vec();
  while (!at_eof()) {
    global();
  }
  return prog;
}

void global() {
  expect("int");

  Node *node = calloc(1, sizeof(Node));
  Token *ident = consume_ident();
  node->name = strndup(ident->str, ident->len);

  // Function Definition
  if (consume("(")) {
    node->kind = ND_FUNC;
    node->params = new_vec();
    lvars = new_vec();
    while (!consume(")")) {
      if (node->params->len > 0) {
        expect(",");
      }
      Var *p = param();
      vec_push(node->params, p);
      vec_push(lvars, p);
    }
    node->body = stmt();
    node->lvars = lvars;
    vec_push(prog->funcs, node);
    return;
  }
}

Var *param() {
  Var *p = calloc(1, sizeof(Var));
  p->typ = typ();
  Token *ident = consume_ident();
  p->name = strndup(ident->str, ident->len);
  p->offset = 8 * (lvars->len + 1);
  return p;
}

Type *typ() {
  expect("int");
  Type *node = calloc(1, sizeof(Type));
  node->typ = TY_INT;
  while (consume("*")) {
    Type *pnode = calloc(1, sizeof(Type));
    pnode->typ = TY_PTR;
    pnode->ptr_to = node;
    node = pnode;
  }
  return node;
}

// stmt = expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
Node *stmt() {
  Node *node;

  if (consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = new_vec();
    while(!consume("}")) {
      vec_push(node->stmts, stmt());
    }
    return node;
  }

  if (consume_keyword(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume_keyword(TK_IF)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume_keyword(TK_ELSE)) {
      node->els = stmt();
    }
    return node;
  }

  if (consume_keyword(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  }

  if (consume_keyword(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    }
    node->body = stmt();
    return node;
  }

  if (startswith(token->str, "int")) {
    Var *lvar = calloc(1, sizeof(Var));
    lvar->typ = typ();
    Token *name = consume_ident();
    lvar->name = strndup(name->str, name->len);
    lvar->offset = 8 * (lvars->len + 1);
    vec_push(lvars, lvar);
    expect(";");
    return NULL;
  }

  node = expr();
  expect(";");
  return node;
}

// expr = equality
Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume_keyword(TK_SIZEOF)) {
    Node *operand = unary();
    if (operand->typ->typ == TY_PTR) {
      return new_node_num(8);
    } else if (operand->typ->typ == TY_INT) {
      return new_node_num(4);
    }
  }
  if (consume("+"))
    return primary();                                    // +x = x
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary()); // -x = 0 - x
  if (consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  return primary();
}

// primary = num
//         | ident ("(" ")")?
//         | "(" expr ")"
Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    if (consume("(")) {
      node->kind = ND_CALL;
      node->name = strndup(tok->str, tok->len);
      // TODO: Implement proper function type inference
      Type *typ = calloc(1, sizeof(Type));
      typ->typ = TY_INT;
      node->typ = typ;
      if (consume(")")) {
        return node;
      }
      node->args = new_vec();
      vec_push(node->args, expr());
      while (consume(",")) {
        vec_push(node->args, expr());
      }
      expect(")");
      return node;
    }
    node->kind = ND_LVAR;
    Var *lvar = find_lvar(tok);
    if (lvar == NULL) {
      error("未定義の変数です → %.*s\n", tok->len, tok->str);
    }
    node->offset = lvar->offset;
    node->typ = lvar->typ;
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}
