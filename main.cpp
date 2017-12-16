#include <iostream> 
#include <cstdio>
#include <vector>
#include "trampoline.h"
#include <cassert>
#include <functional>
#include <memory>


struct struct_test_1 {
    int operator()(int p1, int p2, int p3, int p4, int p5) {
        return 123;
    }
};

void small_easy_test() {
    struct_test_1 tmp;
    trampoline<int (int, int, int, int, int)> t(tmp);
    assert (123 == t.get()(1, 1, 1, 1, 1));
    
    std::cout << "Test 0 passed" << std::endl << std::endl;
}


int sum(int a, int b) {
    return a + b;
}

void small_pointer_test() {
    std::function<char*(int *)> fun = [](int *){ return std::make_shared<char>('a').get(); };
    trampoline<char*(int*)> t(fun);
    assert ('a' == *t.get()(nullptr));
    
    std::cout << "Test 1 passed" << std::endl << std::endl;
}


void small_hard_test() {
    {
        trampoline <int (int, int, int, int, int)>
        t([&] (int p0, int p1, int p2, int p3, int p4) { return p1 + p2 + p3 + p4 + p0; });
        assert (5 == t.get()(1, 1, 1, 1, 1));
        
        std::cout << "int test completed"  << std::endl;
    }
    {
        trampoline <double (double, double, double, double, double)>
        t([&] (double p0, double p1, double p2, double p3, double p4) { return p1 + p2 + p3 + p4 + p0; });
        assert (5.2 == t.get()(1.0, 1.2, 1, 1, 1));
        
        std::cout << "double test completed"  << std::endl;
    }
    {
        trampoline <float (float, float, float, float, float)>
        t([&] (float p0, float p1, float p2, float p3, float p4) { return p1 + p2 + p3 + p4 + p0; });
        float res = 5.2;
        assert (res == t.get()(1.0, 1.2, 1, 1, 1));
        
        std::cout << "float test completed"  << std::endl;
    }
    {
        trampoline <float (int, double, int, float, float)>
        t([&] (int p0, double p1, int p2, float p3, float p4) { return (p1 + p2 + p3 + p4 + p0); });
        float res = 5.2;
        assert (res == t.get()(1.0, 1.2, 1, 1, 1));
        
        std::cout << "int double float test completed"  << std::endl;
    }
    {
        trampoline<int (int&)>
        t([&] (int p0) { return p0; });
        int a = 1;
        assert (1 == t.get()(a));
        
        std::cout << "& test completed"  << std::endl;
    }
    
    std::cout << "Test 2 passed" << std::endl << std::endl;
}

void large_easy_test() {
    {
        trampoline <long long (int, int, int, int, int, int, int, int)>
        t([&] (int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7; });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
        
        std::cout << "int test completed"  << std::endl;
    }
    {
        trampoline <long long (double, double, double, double, double, double, double, double)>
        t([&] (double p0, double p1, double p2, double p3, double p4, double p5, double p6, double p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7; });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
        
        std::cout << "double test completed"  << std::endl;
    }
    {
        trampoline <long long (float, float, float, float, float, float, float, float)>
        t([&] (float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7; });
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
        
        std::cout << "float test completed"  << std::endl;
    }
    {
        trampoline <long long (double, int, float, int, int, double, double, float)>
        t([&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;});
        assert (8 == t.get()(1, 1, 1, 1, 1, 1, 1, 1));
        
        std::cout << "int double float test completed"  << std::endl;
    }
    {
        trampoline <long long (double&, int&, float&, int, int, double, double, float&)>
        t([&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;});
        double a = 1;
        int b = 1;
        float c = 1, d = 1;
        assert (8 == t.get()(a, b, c, 1, 1, 1, 1, d));
        
        std::cout << "& int double float test completed"  << std::endl;
    }
    
    std::cout << "Test 3 passed" << std::endl << std::endl;
}

void large_hard_test() {
    {
        trampoline <float (double, int, float, int, int, double, double, float)>
        t0([&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;});
        auto p0 = t0.get();
        
        trampoline <float (double, int, float, int, const int&, double&, double, float&)>
        t1([&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7)
        { return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;});
        const int a = 1;
        double b = 3.7;
        float c = 4.1;
        auto p1 = t1.get();
        assert ((float)(p0(1, 1, 1, 1, 1, 1, 1, 1) + 103.8) == p1(1, 2, 100, -1, a, b, 1, c));
        
        std::cout << "int double float test completed"  << std::endl;
    }
    {
        trampoline <float (double, int, float, int, int, double, double, float)>
        t([&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7)
        { return p2;});
        auto p = t.get();
        assert(p(1, 2, 3, 4, 5, 6, 7, 8) == 3);
        
        std::cout << "parms sequence test completed" << std::endl;
    }
    {
        typedef std::string st;
        std::function<int(st *c1, st *c2, st *c3, st *c4, st * c5, st *c6, st &c7, st *c8, st &c9)>
        fun = [](st *c1, st *c2, st  *c3, st *c4, st * c5, st *c6, st &c7, st *c8, st &c9)
        { return std::string("")
            .append(*c1)
            .append(*c2)
            .append(*c3)
            .append(*c4)
            .append(*c5)
            .append(*c6)
            .append(c7).append(*c8).append(c9).size(); };
        std::string sample = "test";
        char const *sp = sample.c_str();
        std::vector<std::string> v;
        for(size_t i = 0; i < 9; i++) {
            v.push_back(std::string(sp + i));
        }
        int sz = fun(&v[0], &v[1], &v[2], &v[3], &v[4], &v[5], v[6], &v[7], v[8]);
        int tr_sz = trampoline<int(st *c1, st *c2, st *c3, st *c4, st *c5, st *c6, st& c7, st *c8, st& c9)>(fun)
        .get()(&v[0], &v[1], &v[2], &v[3], &v[4], &v[5], v[6], &v[7], v[8]);
        assert(tr_sz == sz);
        
        std::cout << "* & test completed" << std::endl;
    }
    
    std::cout << "Test 4 passed" << std::endl << std::endl;
}

void methods_test () {    
    {
        trampoline <int(int, int)> t0 = [&] (int p0, int p1) { return p0 + p1; };
        assert (2 == t0.get()(1, 1));
        trampoline<int(int,int)> t1(std::move(t0));
        assert (2 == t1.get()(1, 1));
        trampoline <int(int, int)> t2 = [&] (int p0, int p1) { return p0 - p1; };
        swap(t1, t2);
        assert(t1.get()(1, 1) == 0);
        assert(t2.get()(1, 1) == 2);
    }
    

    std::cout << "Test 5 passed" << std::endl << std::endl;
}

void giga_hard_test() {

    {
        trampoline <int (float, float, float, float, float, float, float, float, float)>
        t([&] (float p2, float p7, float p8, float p9, float p10, float p11, float p12, float p13, float p14)
        { return p7;});
        auto p = t.get();
        //std::cout << p(6, 7.1, 8.8, 10, 8.8, 10, 8.8, 10, 8.8) << "\n";
        assert(p(6, 7.1, 8.8, 10, 8.8, 10, 8.8, 10, 8.8) == 7);
        
        std::cout << "giga test 1 completed" << std::endl;
    }


    {
        trampoline <float (float, float, float, float, float, float, float, float, float, int)>
        t([&] (float p2, float p7, float p8, float p9, float p10, float p11, float p12, float p13, float p14, int i1)
        { return p7;});
        float res = 7.1;
        assert(t.get()(6, 7.1, 8.8, 10, 8.8, 10, 8.8, 10, 8.8, 1) == res);
        
        std::cout << "giga test 2 completed" << std::endl;
    }
}

int main()
{
    small_easy_test();
    small_pointer_test();
    small_hard_test();
    large_easy_test();
    large_hard_test();
    methods_test();
    giga_hard_test();
    std::cout << std::endl << "All passed" << std::endl << std::endl;
    return 0;
}



