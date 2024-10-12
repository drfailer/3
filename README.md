# 3

3 is a very basic programming language in which all the keywords are 3
characters long.

## Current state

- Language transpiled in python.
- Use `flex` and `bison` as lexer/parser generator.
- Use a very basic preprocessor.

## TODO

- Introduce the `dyn` keyword for dynamic arrays.
- Add the `ref` keyword to pass variables by reference.
- Implement `obj` (structs)

## Syntax example

```3
~~~ int a -> error: global variables are not allowed

int fib(int n) bgn
    int fnn
    int fnn1
    int fnn2
    set(fnn2, 1)
    set(fnn1, 1)

    cnd ieq(n, 2) bgn
        ret n
    end els bgn
        int i

        for i rng(2, n, 1) bgn
            set(fnn, add(fnn1, fnn2))
            set(fnn2, fnn1)
            set(fnn1, fnn)
        end
        ret fnn
    end
end

~~~ entry point
nil main() bgn
    int n
    shw("Enter a number:\n")
    ipt(n)
    shw("fib of n is: ")
    set(n, fib(n))
    shw(n)
    shw("\n")
end
```
