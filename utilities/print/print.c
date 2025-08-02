#include <stdio.h>

int print_str(const char *val) {
    int ret = printf("%s", val);
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
