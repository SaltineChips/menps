
#pragma once

#include "command_queue.hpp"
#include "device/mpi/command/mpi_completer.hpp"
#include <mgbase/thread.hpp>

namespace mgcom {
namespace mpi1 {

class command_consumer
    : public virtual command_queue
{
public:
    command_consumer()
        : finished_{false}
        , completer_()
    {
        completer_.initialize();
        
        th_ = mgbase::thread(starter{*this});
    }
    
    virtual ~command_consumer()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        // Join the running thread.
        th_.join();
        
        completer_.finalize();
    }
    
    command_consumer(const command_consumer&) = delete;
    command_consumer& operator = (const command_consumer&) = delete;
    
protected:
    mpi::mpi_completer& get_completer() MGBASE_NOEXCEPT
    {
        return completer_;
    }
    
private:
    struct starter
    {
        command_consumer& self;
        
        void operator() () {
            self.loop();
        }
    };
    
    void loop()
    {
        while (MGBASE_LIKELY((! finished_)))
        {
            // Check the queue.
            if (command* const cmd = this->peek())
            {
                // Call the closure.
                const bool succeeded = execute(*cmd);
                
                if (MGBASE_LIKELY(succeeded)) {
                    MGBASE_LOG_DEBUG("msg:Operation succeeded.");
                    this->pop();
                }
                else {
                    MGBASE_LOG_DEBUG("msg:Operation failed. Postponed.");
                }
            }
            
            // Do polling.
            completer_.poll_on_this_thread();
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool execute(const command& cmd)
    {
        return cmd.func(cmd.arg);
    }
    
    bool finished_;
    mgbase::thread th_;
    
    mpi::mpi_completer completer_;
};

} // namespace mpi1
} // namespace mgcom

