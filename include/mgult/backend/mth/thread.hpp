
#pragma once

#include "mth.hpp"

#include <mgbase/unique_ptr.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/tuple.hpp>

namespace mgult {
namespace backend {
namespace mth {

namespace detail {

template <typename F, typename... Args>
struct thread_functor
{
    // Note: This constructor is for old compilers.
    template <typename F2, typename Args2>
    /*implicit*/ thread_functor(F2&& f2, Args2&& args2)
        : func(mgbase::forward<F2>(f2))
        , args(mgbase::forward<Args2>(args2))
    { }
    
    F                       func;
    mgbase::tuple<Args...>  args;
    
    static void* invoke(void* const p)
    {
        const auto self = static_cast<thread_functor*>(p);
        
        mgbase::apply(self->func, self->args);
        
        delete self;
        
        return MGBASE_NULLPTR;
    }
};

struct thread_deleter
{
    void operator() (const myth_thread_t th) const
    {
        if (th)
        {
            MGBASE_LOG_FATAL(
                "msg:Thread was neither joined nor detached.\t"
                "id:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(th)
            );
            
            // Terminate this program.
            std::terminate();
        }
    }
};

} // namespace detail

class thread
{
    typedef detail::thread_deleter                          deleter_type;
    typedef mgbase::unique_ptr<myth_thread, deleter_type>   thread_ptr_type;
    
public:
    thread() MGBASE_NOEXCEPT = default;
    
    template <typename F, typename... Args>
    explicit thread(F&& f, Args&&... args)
    {
        typedef detail::thread_functor<
            typename mgbase::decay<F>::type
        ,   typename mgbase::decay<Args>::type...
        >
        functor_type;
        
        const auto fp = &functor_type::invoke;
        
        const auto p =
            new functor_type{
                mgbase::forward<F>(f)
            ,   mgbase::make_tuple(
                    mgbase::forward<Args>(args)...
                )
            };
        
        t_.reset(
            myth_create(fp, p)
        );
    }
    
    thread(const thread&) = delete;
    thread& operator = (const thread&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_1(thread, t_)
    
    bool joinable() MGBASE_NOEXCEPT {
        return static_cast<bool>(t_);
    }
    
    void join()
    {
        const auto th = t_.release();
        
        void* r;
        myth_join(th, &r);
        
        // TODO: r is ignored
    }
    
    void detach()
    {
        const auto th = t_.release();
        
        myth_detach(th);
    }
    
private:
    thread_ptr_type t_;
};

} // namespace mth
} // namespace backend
} // namespace mgult

