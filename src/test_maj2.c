#undef NDEBUG
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "maj2_random.h"

typedef unsigned long long ullong;

void sncatprintf(char *buf, size_t buflen, const char* fmt, int64_t n)
{
    size_t len = strlen(buf);
    snprintf(buf+len, buflen-len, fmt, n);
}

char* format_comma(int64_t n)
{
    static char buf[65];

    buf[0] = 0;
    int n2 = 0;
    int scale = 1;
    if (n < 0) {
        buf[0] = '-';
        buf[1] = 0;
        n = -n;
    }
    while (n >= 1000) {
        n2 = n2 + scale * (n % 1000);
        n /= 1000;
        scale *= 1000;
    }
    sncatprintf(buf, sizeof(buf), "%d", n);
    while (scale != 1) {
        scale /= 1000;
        n = n2 / scale;
        n2 = n2  % scale;
        sncatprintf(buf, sizeof(buf), ",%03d", n);
    }
    return buf;
}

char* format_rate(double rate)
{
    static char buf[65];
    char unit = ' ';
    double divisor = 1;
    if (rate > 1e9) {
        divisor = 1e9; unit = 'G';
    } else if (rate > 1e6) {
        divisor = 1e6; unit = 'M';
    } else if (rate > 1e6) {
        divisor = 1e3; unit = 'K';
    }
    snprintf(buf, sizeof(buf), "%.1f %c", rate / divisor, unit);
    return buf;
}

vec2 f1(ullong i, ullong j) { return (vec2){ 0.0f, (float)i / (float)j }; }
vec2 f2(ullong i, ullong j) { return (vec2){ (float)i / (float)j, 0.0f }; }
vec2 f3(ullong i, ullong j) { return (vec2){ (float)i / (float)j, (float)i / (float)j }; }

void test_maj(const char* name, int NROUNDS, ullong count, ullong range, vec2(*f)(ullong,ullong))
{
    vec2 sum = { 0.f, 0.f };
    vec2 var = { 0.f, 0.f };
    int debug = 0;

    fflush(stdout);
    for (ullong i = 0; i < count; i++) {
        vec2 p = f(i, range);
        vec2 q = maj_random(p, NROUNDS);
        sum.a[0] += q.a[0];
        sum.a[1] += q.a[1];
        var.a[0] += (q.a[0]-.5f)*(q.a[0]-.5f);
        var.a[1] += (q.a[1]-.5f)*(q.a[1]-.5f);
    }

    printf("%-32s%12s%12.5f%12.5f%12.5f%12.5f%12.5f%12.5f\n",
        name, format_comma(count), sum.a[0]/count, sum.a[1]/count,
        var.a[0]/count, var.a[1]/count, sqrt(var.a[0]/count), sqrt(var.a[1]/count));
}

void test_header(const char *name)
{
    printf("%-32s%12s%12s%12s%12s%12s%12s%12s\n",
        name, "count", "mean(x)", "mean(y)",
        "variance(x)", "variance(y)",
        "std-dev(x)", "std-dev(y)");
}

void run_all_tests(int NROUNDS, ullong i, ullong j)
{
    char name[32];
    snprintf(name, sizeof(name), "maj_random (NROUNDS=%d)", NROUNDS);
    printf("\n");
    test_header(name);
    test_maj("(             0, (0 - 1M)/8M )", NROUNDS, i, j, f1);
    test_maj("(             0, (0 - 8M)/8M )", NROUNDS, j, j, f1);
    test_maj("( (0 - 1M)/8M ),           0 )", NROUNDS, i, j, f2);
    test_maj("( (0 - 8M)/8M ),           0 )", NROUNDS, j, j, f2);
    test_maj("( (0 - 1M)/8M ), (0 - 1M)/8M )", NROUNDS, i, j, f3);
    test_maj("( (0 - 8M)/8M ), (0 - 8M)/8M )", NROUNDS, j, j, f3);
    printf("\n");
}

int main(int argc, char **argv)
{
    ullong i = argc >= 2 ? atoi(argv[1]) : 1000000;
    ullong j = argc >= 3 ? atoi(argv[2]) : 8000000;
    run_all_tests(2, i, j);
    run_all_tests(4, i, j);
    run_all_tests(6, i, j);
    run_all_tests(8, i, j);
    return 0;
}
