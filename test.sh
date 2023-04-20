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

assert 47 'main() { return 5 + 6 * 7; }'
assert 4 'main() { return (3 + 5) / 2; }'

assert 5 'add(x, y) { return x + y; } main() { return add(2, 3); }'
assert 100 'addd(c, x) { if (c > 0) { return x + addd(c - 1, x); } else { return 0; } } main() { addd(20, 5); }'

assert 144 'fib(n) { if (n >= 2) { return fib(n - 1) + fib(n - 2); } return 1; } main() { return fib(11); }'

assert 128 'baibai(acc) { while (acc < 100) { acc = acc + acc; } return acc; } main() { return baibai(1); }'

echo OK

