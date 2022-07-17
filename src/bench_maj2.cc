#undef NDEBUG
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <climits>
#include <cinttypes>
#include <cassert>

#include "maj2_random.h"

#include <vector>
#include <string>
#include <chrono>

using namespace std::chrono;

typedef long long llong;

static const char* format_unit(llong n)
{
    static char buf[32];
    if (n % 1000000000 == 0) {
        snprintf(buf, sizeof(buf), "%lluG", n / 1000000000);
    } else if (n % 1000000 == 0) {
        snprintf(buf, sizeof(buf), "%lluM", n / 1000000);
    } else if (n % 1000 == 0) {
        snprintf(buf, sizeof(buf), "%lluK", n / 1000);
    } else {
        snprintf(buf, sizeof(buf), "%llu", n);
    }
    return buf;
}

static const char* format_binary(llong n)
{
    static char buf[32];
    if (n % (1<<30) == 0) {
        snprintf(buf, sizeof(buf), "%lluGi", n / (1<<30));
    } else if (n % (1<<20) == 0) {
        snprintf(buf, sizeof(buf), "%lluMi", n / (1<<20));
    } else if (n % (1<<10) == 0) {
        snprintf(buf, sizeof(buf), "%lluKi", n / (1<<10));
    } else {
        snprintf(buf, sizeof(buf), "%llu", n);
    }
    return buf;
}

static const char* format_comma(llong n)
{
    static char buf[32];
    char buf1[32];

    snprintf(buf1, sizeof(buf1), "%llu", n);

    llong l = strlen(buf1), i = 0, j = 0;
    for (; i < l; i++, j++) {
        buf[j] = buf1[i];
        if ((l-i-1) % 3 == 0 && i != l -1) {
            buf[++j] = ',';
        }
    }
    buf[j] = '\0';

    return buf;
}

static std::string format_string(const char* fmt, ...)
{
    std::vector<char> buf;
    va_list args1, args2;
    int len, ret;

    va_start(args1, fmt);
    len = vsnprintf(NULL, 0, fmt, args1);
    assert(len >= 0);
    va_end(args1);

    buf.resize(len + 1);
    va_start(args2, fmt);
    ret = vsnprintf(buf.data(), buf.capacity(), fmt, args2);
    assert(len == ret);
    va_end(args2);
    
    return std::string(buf.data(), len);
}

static void print_header()
{
	printf("\n");
    printf("%-16s %8s %8s %13s %10s\n",
        "benchmark",
        "size(W)",
        "time(ns)",
        "word/sec",
        "MiB/s"
    );

    printf("%-16s %8s %8s %13s %10s\n",
        "----------------",
        "--------",
        "--------",
        "-------------",
        "----------"
    );
}

static void print_footer()
{
	printf("\n");
}

template <typename P>
static void print_result(std::string name, size_t n, P p1, P p2)
{
    double t = (double)duration_cast<nanoseconds>(p2 - p1).count();
    double ns_byte = t / n;
    double time_nsec = t / n;
    double word_sec = n * (1e9 / t);
    double mib_sec = n * sizeof(float) * (1e9 / t) / (1024*1024);
    printf("%-16s %8s %8.2f %13s %10.3f\n",
        name.c_str(),
        format_unit(n),
        time_nsec,
        format_comma((llong)word_sec),
        mib_sec
    );
}

static vec2 bench_maj2_random(size_t n)
{
    vec2 r = { 0, 0 };
    auto t1 = high_resolution_clock::now();
    for (size_t i = 0; i < n; i++) {
        vec2 x = maj2_random({ i / (float)n, i / (float)n });
        r.a[0] += x.a[0];
        r.a[1] += x.a[1];
    }
    auto t2 = high_resolution_clock::now();
    print_result("maj2_random", n, t1, t2);
    return r;
}

int main(int argc, char **argv)
{
    print_header();
    vec2 r = bench_maj2_random(1000000);
    print_footer();
    printf("r={%f,%f}\n", r.a[0], r.a[1]);
}
