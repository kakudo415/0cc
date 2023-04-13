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

echo OK

