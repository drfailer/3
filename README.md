# Prog

This project was originally a school project. The purpose was to create a simple
programming language that had to be transpiled in python. The language is
statically typed and offer the possibility to create variables, arrays and
functions. The language has some special builtin functions for arithmetic
operations or a `print` and a `read` function for reading or printing on the
terminal.

To create this language, I used the tools `flex` and `bison` to generate an
efficient parser. The parser code is used to create an AST which is made of C++
object. Each node of the AST has a compile method that is used to translate the
element in python.


## Syntax example

```
fn fib(int n) -> int {
  int fnn;
  int fnn1;
  int fnn2;
  set(fnn2, 1);
  set(fnn1, 1);

  if (ieq(n, 2)) {
    return n;
  }
  else {
    int i;

    for i in range(2, n, 1) {
      set(fnn, add(fnn1, fnn2));
      set(fnn2, fnn1);
      set(fnn1, fnn);
    }
    return fnn;
  }
}

fn main() {
  int n;
  print("Enter a number:\n");
  read(n);
  print("fib of n is: ");
  set(n, fib(n));
  print(n);
  print("\n");
}
```
