
#include <mgult/sm.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

mgult::scheduler_ptr g_s;

// Fibonacci

typedef mgbase::uint64_t    fib_int_t;

struct fib_func
{
    fib_int_t*  ret;
    fib_int_t   n;
    
    void operator () ()
    {
        if (n == 0 || n == 1) {
            *ret = n;
            return;
        }
        
        fib_int_t r1{};
        fib_int_t r2{};
        
        mgult::sm::thread th{ *g_s, fib_func{ &r1, n-1 } };
        
        fib_func{ &r2, n-2 }();
        
        th.join();
        
        *ret = r1 + r2;
    }
};

fib_int_t g_result = 0;
fib_int_t g_arg_n = 0;

void start_fib()
{
    fib_func{ &g_result, g_arg_n }();
}

} // unnamed namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    g_arg_n = static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    
    g_s = mgult::make_sm_scheduler();
    
    mgbase::stopwatch sw;
    sw.start();
    
    g_s->loop(start_fib);
    
    std::cout << "fib(" << g_arg_n << ") = " << g_result
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    g_s.reset();
    
    return 0;
}

