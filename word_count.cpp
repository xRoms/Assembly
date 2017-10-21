#include <bits/stdc++.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <tmmintrin.h>

using namespace std;


const int ALIGN = 16;

size_t linearly(string s, size_t n) {
    int ans = 0;
    for (size_t i = 0; i < n ; i++) {
        if (s[i] != ' ' && (s[i - 1] == ' ' || i == 0)) {
            ++ans;
        }
    }
    return ans;
}

void recount_flush(size_t &res, __m128i &a) {
    for (int i = 0; i < ALIGN; i++) {
        res += (int)*((uint8_t*)&a + i);
    }
    a = _mm_setzero_si128();
}

size_t count(const char* str, size_t size)
{
    const __m128i SPACES = _mm_set_epi8(' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                             ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    const __m128i ONES = _mm_set_epi8(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    
    size_t result = 0;

    char last = ' ';
    for (;((size_t)(str) % ALIGN) != 0 && (size > 0); size--) {
        if ((last != ' ') && (*str == ' ')) {
            result++;
        }
        //cout << "last " << last << "  cur " << *str <<"\n";
        last = *str;
        str = str + 1;
    }

    if (last != ' ' && (*str) == ' ') {
        result++;
    }
    if (result == 0 && (*str) != ' ') {
        result++;
    }
    if (size >= 2 * ALIGN) {
        __m128i count = _mm_setzero_si128();
        __m128i prev;
        __m128i curr = _mm_load_si128((__m128i *)str);
        curr = _mm_cmpeq_epi8(curr, SPACES);
        int iter = 0;
        for (;size >= 2 * ALIGN; size -= ALIGN) {
            str = str + ALIGN;
            prev = curr;
            curr = _mm_cmpeq_epi8(_mm_load_si128((__m128i *)str), SPACES);
            __m128i shifted = _mm_alignr_epi8(curr, prev, 1);
            count = _mm_add_epi8(count, _mm_and_si128(_mm_andnot_si128(shifted, prev), ONES));
            iter++;
            if (iter == 255) {
                recount_flush(result, count);
                iter = 0;
            }
        }
        recount_flush(result, count);
    }
    //cout << "after fast " << result << " size = " << size << "\n";
    last = *str;
    if (*str != ' ') {
        result--;
    }
    bool notspace = false;
    for (; size > 0; size--) {
        if (*str != ' ') {
            notspace = true;
        }
        if (*str == ' ' && last != ' ') {
            result++;
            notspace = false;
        }
        last = *str;
        str++;
    }
    if (notspace) {
        result++;
    }
    return result;

}

string test() {
    std::string s = "";
    int len = 1000;
    for (int i = 0; i < len; ++i) {
        int k = rand() % 2;
        if (k) {
            s += "A";
        } else {
            s += " ";
        }
    }
    return s;
}

int main() {
    srand(time(0));
    for (int i = 0; i < 1000; i++) {
        std::string s = test();
        const char* st = s.c_str();
        size_t right_ans = linearly(s, (int)s.size());
        size_t my_ans = count(st, s.size());
        cout << "right: " << right_ans << "\n";
        cout << "my: " << my_ans << "\n";
        if (right_ans != my_ans) {
            cout << ":(\n";
            return 0;
        }
        cout << "OK\n";
    }
    return 0;
}
