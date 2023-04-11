# 0cc

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)の写経。

## 実行・テスト

詳しくは、[付録3：Dockerを使った開発環境の作成](https://www.sigbus.info/compilerbook#docker)。

### Docker imageの作成

```sh
docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
```

### Compilerbookコンテナ内でのコマンド実行

```sh
docker run --rm compilerbook {コマンド}
```

- `--rm`: コマンド実行後コンテナを削除

### Compilerbookコンテナ内でビルド・テスト

```sh
docker run --rm -v {ホストのディレクトリ}:{コンテナ内のディレクトリ} -w {コンテナ内のディレクトリ} compilerbook make test
```

- `-v`: ホストのディレクトリをコンテナにマウントする
- `-w`: コマンドを実行するディレクトリを指定する

## （現時点の）文法

```ebnf
expr    = mul ("+" mul | "-" mul)*
mul     = unary ("*" unary | "/" unary)*
unary   = ("+" | "-")? primary
primary = num | "(" expr ")"
```
