
#pragma once

#include "mutex.hpp"

namespace mgult {
namespace backend {
namespace mth {

struct condition_variable_error { };

class condition_variable
{
    typedef mutex   mutex_type;
    
public:
    condition_variable()
        : cond_(MYTH_COND_INITIALIZER)
    { }
    
    condition_variable(const condition_variable&) = delete;
    condition_variable& operator = (const condition_variable&) = delete;
    
    ~condition_variable()
    {
        if (myth_cond_destroy(&cond_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void wait(mgbase::unique_lock<mutex_type>& lk)
    {
        if (!lk.owns_lock())
            throw condition_variable_error();
        
        const auto mtx = lk.mutex();
        if (mtx == MGBASE_NULLPTR)
            throw condition_variable_error();
        
        const auto mtx_ptr = mtx->native_handle();
        
        const auto ret = myth_cond_wait(&cond_, mtx_ptr);
        if (ret != 0)
            throw condition_variable_error();
    }
    
    template <typename Predicate>
    void wait(mgbase::unique_lock<mutex_type>& lc, Predicate pred)
    {
        while (!pred())
        {
            wait(lc);
        }
    }
    
    void notify_one()
    {
        if (myth_cond_signal(&cond_) != 0)
            throw condition_variable_error();
    }
    
    void notify_all()
    {
        if (myth_cond_broadcast(&cond_) != 0)
            throw condition_variable_error();
    }
    
private:
    myth_cond_t cond_;
};

} // namespace mth
} // namespace backend
} // namespace mgult

