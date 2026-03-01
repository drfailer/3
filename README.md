# 3

3 is a very basic programming language in which all the keywords are 3
characters long.

```3
int main() bgn
    shw("Hello, World!")
    ret 0
end
```

## Current state

- Only works on `linux-x86_64`.
- Use `flex` and `bison` as lexer/parser generator.
- Compiles to `x86_64` assembly (use GAS with the intel syntax under the hood,
  but I may switch to `fasm` at some point).
- Links usint `ld`.
- C ffi: 3 can link to C libraries, however, the current version of 3 doesn't
  have structs or pointers, and 3 primitive types are all 8 bytes long (was
  done to simplify writing the assembly, but will change in the future) which
  can cause issues with arrays.
- features:
  - primitive types: `chr`, `int`, `flt`, `str` (these types will change).
  - control structures: `cnd/otw`, `for`, `whl`
  - functions (definition order doesn't matter).
  - arrays and strings are not fully functional.

## Current bugs/limitations

- Variables allocated on the stack in blocks are not released (unless the block is a function block).
- `use` statement use the wrong path in sub-directories.
- The preprocessor should be refactored / removed.
- All types are 8 bytes long.
- Arrays and strings are not fully functional.
- GAS does not fully support the intel syntax, which forces to use a lot of workaround in the assembly.
