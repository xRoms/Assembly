#include <ctime>
#include <cstdlib>
#include <iostream>
#include <emmintrin.h>
#include <cassert>

using namespace std;

void copy_asm(char *dst, char const *src, size_t size)
{
    const size_t ALIGN = 16;

    for (; (size > 0) && (static_cast<size_t>(*dst) % ALIGN) != 0; size--) {
        *dst = *src;
        src++;
        dst++;
    }

    for (; size >= ALIGN; size -= ALIGN) {
        __m128i tmp;
        __asm__ volatile (
            "movdqu\t     (%[aSRC]), %[aTMP]\n"
            "movdqu\t      %[aTMP], (%[aDST])\n"
            : [aTMP] "=x"(tmp)
            : [aSRC] "r"(src), [aDST] "r"(dst)
            : "memory");
        dst += ALIGN;
        src += ALIGN;
    }

    for (;size > 0; size--) {
        *dst = *src;
        src++;
        dst++;
    }
    _mm_sfence();
}

void copy_asm(void* dst, void const* src, size_t size) {
    copy_asm(static_cast<char*>(dst), static_cast<char const*>(src), size);
}

int main() {
    srand(time(NULL));
    for (int t = 0; t < 10000; t++) {
            int n = rand() % 10000;
            int a[n];
            int b[n];
            int c[n];
            for (int i = 0; i < n; i++) {
                a[i] = rand();  
                c[i] = a[i];
            }
            copy_asm(b, a, sizeof(a));
            for (int i = 0; i < n; ++i) {
                if ((a[i] != b[i]) || (c[i] != a[i])) {
                    cout << "wrong :c\n";
                    return 0;
                }
            }

    }
    cout << "yesss\n";
    return 0;
}
