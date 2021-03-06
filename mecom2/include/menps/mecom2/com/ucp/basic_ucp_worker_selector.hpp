
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/ucx/ucp/ucp.hpp>

#define MECOM2_UCP_USE_ULT_WORKER_NUM

namespace menps {
namespace mecom2 {

template <typename P>
class basic_ucp_worker_selector
{
    using worker_num_type = typename P::worker_num_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_unique_lock_type = typename ult_itf_type::unique_mutex_lock; // TODO
    
public:
    template <typename Conf>
    explicit basic_ucp_worker_selector(const Conf& conf)
        : max_num_(conf.max_wk_num)
    { }
    
    worker_num_type current_worker_num()
    {
        #ifdef MECOM2_UCP_USE_ULT_WORKER_NUM
        return ult_itf_type::get_worker_num();
        
        #else
        // Load the TLS.
        auto cur_num = cur_num_;
        
        if (MEFDN_UNLIKELY(cur_num == 0)) {
            mutex_unique_lock_type lk(this->mtx_);
            if (this->alloc_num_ >= this->max_num_) {
                // TODO: Better exception class.
                throw std::bad_alloc();
            }
            
            cur_num = ++this->alloc_num_;
            cur_num_ = cur_num;
        }
        
        MEFDN_ASSERT(cur_num >= 1);
        
        return cur_num - 1;
        #endif
    }
    
private:
    #ifndef MECOM2_UCP_USE_ULT_WORKER_NUM
    // Note: Valid numbers start from 1.
    static MEFDN_THREAD_LOCAL worker_num_type cur_num_;
    #endif
    
    mutex_type mtx_;
    worker_num_type alloc_num_ = 0;
    worker_num_type max_num_ = 0;
};

#ifndef MECOM2_UCP_USE_ULT_WORKER_NUM
template <typename P>
MEFDN_THREAD_LOCAL typename P::worker_num_type
basic_ucp_worker_selector<P>::cur_num_ = 0;
#endif

} // namespace mecom2
} // namespace menps

