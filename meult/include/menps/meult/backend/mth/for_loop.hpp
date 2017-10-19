
#pragma once

#include <menps/meult/backend/mth/thread.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace meult {
namespace backend {
namespace mth {

struct parallel_execution_policy { };

namespace /*unnamed*/ {

MEFDN_MAYBE_UNUSED parallel_execution_policy par;

} // unnamed namespace

template <typename I, typename S, typename F>
void for_loop_strided(parallel_execution_policy /*ignored*/, I first, I last, S stride, F func)
{
    const auto diff = last - first;
    
    if (diff <= 0) {
        return;
    }
    else if (diff <= stride) {
        func(first);
    }
    else {
        const auto half = diff / 2 / stride * stride;
        thread t(&for_loop_strided<I, S, F>, par, first, first + half, stride, func);
        //for_loop_strided(par, first, first + half, stride, func);
        for_loop_strided(par, first + half, last, stride, func);
        t.join();
    }
}

template <typename ExecutionPolicy, typename I, typename F>
void for_loop(ExecutionPolicy&& exec, I first, I last, F func)
{
    for_loop_strided(mefdn::forward<ExecutionPolicy>(exec), first, last, 1, func);
}

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

