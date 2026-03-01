/*
 * Temporary glue code to convert a 3 string ([len][ptr]), into a C string. Note
 * that since strings are initialized usin assembly strings from Gas (with
 * .string type), they are also null terminated, therefore, we can just return
 * the pointer without additional allocation.
 */

char *to_c_string(unsigned long *str3) {
    char *cstr = (char*)(*(str3 - 1));
    return cstr;
}

/* The previous code is equivalent to following xD (which works fine if compiled
 * without optimization):
 * char const *to_c_string(char const *str) {
 *     asm(
 *         ".intel_syntax;"
 *         "mov %rax, [%rdi - 8];"
 *         "mov %rsp, %rbp;"
 *         "pop %rbp;"
 *         ".att_syntax;"
 *         "ret"
 *     );
 *     return str;
 * }
 */
