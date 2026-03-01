#include <stdio.h>

// this function does not work with standard strings
int print_str(unsigned long *str3) {
    char *cstr = (char*)(*(str3 - 1));
    int ret = printf("%s", cstr);
    if (ret == 0) {
        return ret;
    }
    fflush(stdout);
    return 0;
}

int print_flt(double val) {
    int ret = printf("%lf", val);
    if (ret == 0) {
        return ret;
    }
    fflush(stdout);
    return 0;
}

int print_int(long val) {
    int ret = printf("%ld", val);
    if (ret == 0) {
        return ret;
    }
    fflush(stdout);
    return 0;
}
