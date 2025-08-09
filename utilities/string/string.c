char const *to_c_string(char const *str) {
    asm(
        ".intel_syntax;"
        "lea %rax, [%rdi - 8];"
        "mov %rax, [%rax];"
        "mov %rsp, %rbp;"
        "pop %rbp;"
        ".att_syntax;"
        "ret"
    );
    return str;
}
