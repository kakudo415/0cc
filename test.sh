#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./0cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

assert 47 'int main() { return 5 + 6 * 7; }'
assert 4 'int main() { return (3 + 5) / 2; }'

assert 5 'int add(int x, int y) { return x + y; } int main() { return add(2, 3); }'
assert 100 'int addd(int c, int x) { if (c > 0) { return x + addd(c - 1, x); } else { return 0; } } int main() { return addd(20, 5); }'

assert 144 'int fib(int n) { if (n >= 2) { return fib(n - 1) + fib(n - 2); } return 1; } int main() { return fib(11); }'

assert 128 'int baibai(int acc) { while (acc < 100) { acc = acc + acc; } return acc; } int main() { return baibai(1); }'

assert 105 '
int countsum(int c) {
  int sum;
  sum = 0;
  int i;
  for (i = 0; i <= c; i = i + 1) {
    sum = sum + i;
  }
  return sum;
}
int main() {
  return countsum(14);
}
'

assert 3 'int main() { int x; x = 3; int y; y = &x; return *y; }'

echo OK

