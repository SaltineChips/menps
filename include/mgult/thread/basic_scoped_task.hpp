
#pragma once

#include <mgbase/utility/forward.hpp>
#include <mgbase/tuple/apply.hpp>

namespace mgult {

namespace detail {

template <typename T>
struct scoped_task_functor
{
    T* result;
    
    template <typename Func, typename... Args>
    void operator() (Func&& func, Args&&... args)
    {
        *result = mgbase::forward<Func>(func)(mgbase::forward<Args>(args)...);
    }
};

template <>
struct scoped_task_functor<void>
{
    template <typename Func, typename... Args>
    void operator() (Func&& func, Args&&... args)
    {
        mgbase::forward<Func>(func)(mgbase::forward<Args>(args)...);
    }
};

} // namespace detail

template <typename Policy, typename T>
class basic_scoped_task
{
    typedef typename Policy::derived_type   derived_type;
    typedef typename Policy::thread_type    thread_type;
    
public:
    template <typename Func, typename... Args>
    explicit basic_scoped_task(Func&& func, Args&&... args)
        : result_{}
        , th_(
            detail::scoped_task_functor<T>{ &result_ }
        ,   mgbase::forward<Func>(func)
        ,   mgbase::forward<Args>(args)...
        )
    { }
    
    basic_scoped_task(const basic_scoped_task&) = delete;
    
    basic_scoped_task& operator = (const basic_scoped_task&) = delete;
    
    T& get()
    {
        wait();
        return result_;
    }
    
    void wait()
    {
        if (th_.joinable()) {
            th_.join();
        }
    }
    
private:
    T           result_;
    thread_type th_;
};

template <typename Policy>
class basic_scoped_task<Policy, void>
{
    typedef typename Policy::derived_type   derived_type;
    typedef typename Policy::thread_type    thread_type;
    
public:
    template <typename Func, typename... Args>
    explicit basic_scoped_task(Func&& func, Args&&... args)
        : th_(
            detail::scoped_task_functor<void>{ }
        ,   mgbase::forward<Func>(func)
        ,   mgbase::forward<Args>(args)...
        )
    { }
    
    basic_scoped_task(const basic_scoped_task&) = delete;
    
    basic_scoped_task& operator = (const basic_scoped_task&) = delete;
    
    void get()
    {
        wait();
    }
    
    void wait()
    {
        if (th_.joinable()) {
            th_.join();
        }
    }
    
private:
    thread_type th_;
};

} // namespace mgult

