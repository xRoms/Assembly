#ifndef ASM3_TRAMPOLINE_H
#define ASM3_TRAMPOLINE_H

#include <cstdlib>
#include <sys/mman.h>
#include <xmmintrin.h>

template<typename ... Args>
struct args;

template<>
struct args<> {
    static const int INT = 0;
    static const int SSE = 0;
};

template<typename First, typename ... Args>
struct args<First, Args ...> {
    static const int INT = args<Args ...>::INT + 1;
    static const int SSE = args<Args ...>::SSE;
};

template<typename ... Args>
struct args<double, Args ...> {
    static const int INT = args<Args ...>::INT;
    static const int SSE = args<Args ...>::SSE + 1;
};

template<typename ... Args>
struct args<float, Args ...> {
    static const int INT = args<Args ...>::INT;
    static const int SSE = args<Args ...>::SSE + 1;
};

template<typename ... Args>
struct args<__m64, Args ...> {
    static const int INT = args<Args ...>::INT;
    static const int SSE = args<Args ...>::SSE + 1;
};


static void **p = nullptr;
const int SIZE = 128;
const int PAGES_AMOUNT = 1;
const int PAGE_SIZE = 4096;

void alloc() {
    void *mem = mmap(nullptr, PAGE_SIZE * PAGES_AMOUNT,
                     PROT_EXEC | PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);
    p = (void **) mem;

    for (auto i = 0; i < PAGE_SIZE * PAGES_AMOUNT; i += SIZE) {
        auto c = (char *) mem + i;
        *(void **) c = 0;
        if (i != 0) {*(void **) (c - SIZE) = c;}
    }

}

template<typename T>
struct trampoline;

template<typename R, typename... Args>
void swap(trampoline<R(Args...)> &lhs, trampoline<R(Args...)> &rhs);

template<typename T, typename ... Args>
struct trampoline<T(Args ...)> {

    void *func_obj;
    void *code;

    void (*deleter)(void *);
    
    static const int REGS = 6;
    static const int BYTE = 8;

    const char *shifts[REGS] = {
        "\x48\x89\xfe" /*mov rsi rdi*/, "\x48\x89\xf2" /*mov rdx rsi*/,
        "\x48\x89\xd1" /*mov rcx rdx*/, "\x49\x89\xc8" /*mov r8 rcx*/,
        "\x4d\x89\xc1" /*mov r9 r8*/,   "\x41\x51"     /*push r9*/
    };

    void push(char *&p, const char *command) {
        for (const char *i = command; *i; i++) {
            *(p++) = *i;
        }
    }

    void push(char *&p, const char *command, int32_t c) {
        push(p, command);
        *(int32_t *) p = c;
        p += BYTE / 2;
    }

    void push(char *&p, const char *command, void *c) {
        push(p, command);
        *(void **) p = c;
        p += BYTE;
    }


    template<typename F>
    trampoline(F func) {
        func_obj = new F(std::move(func));
        deleter = free_temp<F>;
        if (p == nullptr) {
            alloc();
            if (p == nullptr) {
                code = nullptr;
            }
        }
        code = p;
        p = (void **) *p;

        char *point = (char *) code;

        if (args<Args ...>::INT < REGS) {
            for (int i = args<Args ...>::INT - 1; i >= 0; i--) { push(point, shifts[i]); }
            push(point, "\x48\xbf");
            *(void **) point = func_obj;
            point += BYTE;
            push(point, "\x48\xb8");
            *(void **) point = (void *) &do_call<F>;
            point += BYTE;
            push(point, "\xff\xe0");
        } else {
            int stack_size = BYTE * (args<Args ...>::INT - 5 + std::max(args<Args ...>::SSE - BYTE, 0));

            /* move r11 [rsp] save top of stack address */

            push(point, "\x4c\x8b\x1c\x24");

            /* push sixth arg ti stack, shift other */

            for (int i = 5; i >= 0; i--) {
                push(point, shifts[i]);
            }

            /* moving rax to bottom of stack
                mov rax, rsp 
                add rax, stack_size */

            push(point, "\x48\x89\xe0\x48\x05", stack_size);
            push(point, "\x48\x81\xc4", BYTE);

            /* start of cycle */

            char *label_1 = point;

            /*  compare rax and rsp to know if all arguments were shifted if so go to label_2
                cmp rax,rsp
                je */
            push(point, "\x48\x39\xe0\x74");
            char *label_2 = point;
            point++;
            {
                /*  shifting argument with rdi as temp
                    add rsp, 8  
                    mov rdi, [rsp]
                    mov [rsp-0x8],rdi 
                    cycle
                    jmp
                 */
                push(point, "\x48\x81\xc4\x08");
                point += 3;
                push(point, "\x48\x8b\x3c\x24\x48\x89\x7c\x24\xf8\xeb");
                *point = label_1 - point - 1;
                point++;
            }
            *label_2 = point - label_2 - 1;

            /* returning rsp to top of stack using stored in r11
                mov [rsp], r11                                                  
                sub rsp, stack_size                                               /
            */
            push(point, "\x4c\x89\x1c\x24\x48\x81\xec", stack_size);
            push(point, "\x48\xbf", func_obj);
            push(point, "\x48\xb8", (void*) &do_call<F>);

            /* normalizing stack
               call rax 
               pop r9
               mov r11,[rsp + stack_size]
               mov [rsp],r11                                                      
               return
               ret
            */
            push(point, "\xff\xd0\x41\x59\x4c\x8b\x9c\x24", stack_size - BYTE);
            push(point, "\x4c\x89\x1c\x24\xc3");
        }
    }

    trampoline(trampoline &&other) {
        code = other.code;
        deleter = other.deleter;
        func_obj = other.func_obj;
        other.func_obj = nullptr;
    }

    trampoline(const trampoline &) = delete;

    template<class TR>
    trampoline &operator=(TR &&func) {
        trampoline tmp(std::move(func));
        ::swap(*this, tmp);
        return *this;
    }

    T (*get() const )(Args ... args) {
        return (T(*)(Args ... args)) code;
    }

    void swap(trampoline &other) {
        ::swap(*this, other);
    }

    friend void::swap<>(trampoline &a, trampoline &b);

    ~trampoline() {
        if (func_obj)
        {
            deleter(func_obj);
        }
        *(void **) code = p;
        p = (void **) code;
    }

    template<typename F>
    static T do_call(void *obj, Args ...args) {
        return (*static_cast<F *>(obj))(std::forward<Args>(args)...);
    }

    template<typename F>
    static void free_temp(void *func_obj) {
        delete static_cast<F *>(func_obj);
    }
};

template<typename R, typename... Args>
void swap(trampoline<R(Args...)> &lhs, trampoline<R(Args...)> &rhs) {
    std::swap(lhs.func_obj, rhs.func_obj);
    std::swap(lhs.code, rhs.code);
    std::swap(lhs.deleter, rhs.deleter);
}

#endif //ASM3_TRAMPOLINE_H
