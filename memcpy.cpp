#include <ctime>
#include <cstdlib>
#include <iostream>
#include <emmintrin.h>
#include <cassert>

using namespace std;

void copy_asm(void *dst_, void const *src_, size_t size)
{
    char* dst = static_cast<char*>(dst_);
    const char* src = static_cast<char const*>(src_);
    const size_t ALIGN = 16;

    for (; (size > 0) && ((reinterpret_cast<size_t>(dst)) % ALIGN) != 0; size--) {
        *dst = *src;
        src++;
        dst++;
    }

    for (; size >= ALIGN; size -= ALIGN) {
        __m128i tmp;
        __asm__ volatile (
            "movdqu\t     (%[aSRC]), %[aTMP]\n"
            "movntdq\t      %[aTMP], (%[aDST])\n"
            : [aTMP] "=x"(tmp)
            : [aSRC] "r"(src), [aDST] "r"(dst)
            : "memory");
        dst += ALIGN;
        src += ALIGN;
    }
    
    _mm_sfence();

    for (;size > 0; size--) {
        *dst = *src;
        src++;
        dst++;
    }
}

int main() {
    srand(time(NULL));
    for (int t = 0; t < 10000; t++) {
            int n = rand() % 10000;
            int a[n], b[n], c[n];
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
