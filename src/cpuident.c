#include <stdio.h>
#include <stdlib.h>

#if defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#define HAS_X86_CPUID 1
#include <cpuid.h>
static inline void __x86_cpuidex(int reg[], int level, int count)
{ __cpuid_count(level, count, reg[0], reg[1], reg[2], reg[3]); }
#elif defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
#define HAS_X86_CPUID 1
#define __x86_cpuidex __cpuidex
#endif

#if HAS_X86_CPUID
enum { x86_reg_eax, x86_reg_ebx, x86_reg_ecx, x86_reg_edx };
typedef struct { const char *name; char x, l, r, b; } x86_cpu_feature;
static x86_cpu_feature cpu_feature[] = {
    { "invtsc",        1, 7, x86_reg_edx, 8  },
    { "rdtscp",        1, 1, x86_reg_edx, 27 },
    { "rdrand",        0, 1, x86_reg_ecx, 30 },
    { "rdseed",        0, 7, x86_reg_ebx, 18 },
    { "lzcnt",         1, 1, x86_reg_ecx, 5  },
    { "prefetchw",     1, 1, x86_reg_ecx, 8  },
    { "bmi1",          0, 7, x86_reg_ebx, 3  },
    { "bmi2",          0, 7, x86_reg_ebx, 8  },
    { "fma",           0, 1, x86_reg_ecx, 12 },
    { "cmpxchg16b",    0, 1, x86_reg_ecx, 13 },
    { "movbe",         0, 1, x86_reg_ecx, 22 },
    { "popcnt",        0, 1, x86_reg_ecx, 23 },
    { "tscdt",         0, 1, x86_reg_ecx, 24 },
    { "aesni",         0, 1, x86_reg_ecx, 25 },
    { "f16c",          0, 1, x86_reg_ecx, 29 },
    { "sse",           0, 1, x86_reg_edx, 25 },
    { "sse2",          0, 1, x86_reg_edx, 26 },
    { "ssse3",         0, 1, x86_reg_ecx, 9  },
    { "sse4_1",        0, 1, x86_reg_ecx, 19 },
    { "sse4_2",        0, 1, x86_reg_ecx, 20 },
    { "pclmulqdq",     0, 1, x86_reg_ecx, 1  },
    { "avx",           0, 1, x86_reg_ecx, 28 },
    { "avx2",          0, 7, x86_reg_ebx, 5  },
    { "avx512f",       0, 7, x86_reg_ebx, 16 },
    { "avx512bw",      0, 7, x86_reg_ebx, 30 },
    { "avx512dq",      0, 7, x86_reg_ebx, 17 },
    { "avx512cd",      0, 7, x86_reg_ebx, 28 },
    { "avx512vl",      0, 7, x86_reg_ebx, 31 },
    { "avx512ifma",    0, 7, x86_reg_ebx, 21 },
    { "avx512pf",      0, 7, x86_reg_ebx, 26 },
    { "avx512er",      0, 7, x86_reg_ebx, 27 },
    { "sha",           0, 7, x86_reg_ebx, 29 },
    { "vpclmulqdq",    0, 7, x86_reg_ecx, 10 },
    { "vaes",          0, 7, x86_reg_ecx, 9  },
    { "gfni",          0, 7, x86_reg_ecx, 8  },
    { "avx512vbmi",    0, 7, x86_reg_ecx, 1  },
    { "avx512vbmi2",   0, 7, x86_reg_ecx, 6  },
    { "avx512vnni",    0, 7, x86_reg_ecx, 11 },
    { "avx512bitalg",  0, 7, x86_reg_ecx, 12 },
    { "avx512_4vnniw", 0, 7, x86_reg_edx, 2  },
    { "avx512_4fmaps", 0, 7, x86_reg_edx, 3  },
    { NULL }
};
static void x86_cpuident()
{
    int leaf[2][8][4] = { 0 };
    for (unsigned i = 0; i < 8; i++) {
        if (i == 0 || i <= leaf[0][0][0]) {
            __x86_cpuidex(leaf[0][i], i, 0);
        }
        if (i == 0 || i + (1<<31) <= leaf[1][0][0]) {
            __x86_cpuidex(leaf[1][i], i + (1<<31), 0);
        }
    }
    for (x86_cpu_feature *f = cpu_feature; f->name; f++) {
        int val = !!(leaf[f->x][f->l][f->r] & (1 << f->b));
        printf("%sx86_%s=%d", f != cpu_feature ? ";" : "", f->name, val);
    }
    printf("\n");
}
#endif

int main()
{
#if HAS_X86_CPUID
    x86_cpuident();
#endif
}
