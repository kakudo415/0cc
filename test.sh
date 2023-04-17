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

assert 0 '0;'
assert 55 '55;'

assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'

assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

assert 10 '-10+20;'
assert 21 '-7 * (-3);'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 42 'a=42;'
assert 35 ' b =  35 ;'
assert 23 'z = 10; s = 23;'
assert 81 'x = 3; y = 27; z = x * y;'
assert 64 'n = 2; m = (n + n) * (n - 4) * (-8); m;'

assert 1 'x = 5; y = 2; x > y;'
assert 0 'm = 3; m * m == m;'

assert 10 'ho=10;'
assert 29 'hoge = 44; fuga = 15; hoge - fuga;'
assert 19 'po=2;mu=3*7;mu+(-po);'

assert 17 'return 17;'
assert 5 'return 2 + 3;'
assert 100 'x = 20; y = 5; z = 3; return x * y; return y * z;'

assert 10 'if (1) 10;'
assert 3 'if (0) 10; else 3;'
assert 5 'if (2 > 1) 5; else 3;'
assert 7 'if (3 > 8) 2; else 7;'

assert 15 'acc = 0; while (acc < 15) acc = acc + 1; acc;'
assert 0 'acc = 0; while (0) acc = acc + 3; acc;'

assert 12 'for (i = 0; i < 12; i = i + 1) i; i;'
assert 7 'j = 0; for (; j < 7;) j = j + 1; j;'

assert 21 'a = 0; for (i = 0; i < 7; i = i + 1) for (j = 0; j < 3; j = j + 1) a = a + 1; a;'

assert 128 'acc = 1; while (acc < 100) { acc = acc + acc; } return acc;'
assert 96 'i = 0; acc = 3; while (i < 5) { acc = acc * 2; i = i + 1; } return acc;'

echo OK

