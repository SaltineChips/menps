
#pragma once

#include "device/mpi/mpi_base.hpp"
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/container/circular_buffer.hpp>
#include <mgbase/callback.hpp>
#include <mgbase/logger.hpp>

#include "mpi_completer_base.hpp"

namespace mgcom {
namespace mpi {

class mpi_completer
    : public mpi_completer_base
{
    static const index_t max_completion_count = 256;
    
public:
    void initialize()
    {
        queue_.set_capacity(max_completion_count);
        
        statuses_ = new MPI_Status*[max_completion_count];
        requests_ = new MPI_Request[max_completion_count];
        
        completions_ = new mgbase::callback<void ()>[max_completion_count];
        
        for (index_t i = 0; i < max_completion_count; ++i) {
            requests_[i] = MPI_REQUEST_NULL;
            queue_.push_back(i);
            
            statuses_[i] = MGBASE_NULLPTR;
        }
        
        established_ = 0;
        
        initialized_ = true;
    }
    void finalize()
    {
        MGBASE_ASSERT(initialized_);
        
        for (index_t i = 0; i < max_completion_count; ++i) {
            MPI_Cancel(&requests_[i]);
        }
        requests_.reset();
        
        statuses_.reset();
        completions_.reset();
    }
    
    virtual bool full() const MGBASE_NOEXCEPT MGBASE_OVERRIDE
    {
        // If the index queue is empty, all elements are used.
        return queue_.empty();
    }
    
    virtual void complete(const complete_params& params) MGBASE_OVERRIDE
    {
        MGBASE_ASSERT(initialized_);
        MGBASE_ASSERT(!queue_.empty());
        
        const index_t index = queue_.front();
        queue_.pop_front();
        
        requests_[index] = params.req;
        statuses_[index] = params.status;
        completions_[index] = params.on_complete;
        
        ++established_;
        
        MGBASE_LOG_DEBUG(
            "msg:Added completion of MPI request.\t"
            "index:{}"
        ,   index
        );
    }
    
    void poll_on_this_thread()
    {
        MGBASE_ASSERT(initialized_);
        
        if (queue_.full()) {
            // There are no established requests.
            MGBASE_ASSERT(established_ == 0);
            
            /*MGBASE_LOG_VERBOSE(
                "msg:Free list is full. No request is established.\t"
            );*/
                
            return;
        }
        
        int idx;
        int flag;
        MPI_Status status;
        
        mpi_error::check(
            MPI_Testany(
                max_completion_count    // count
            ,   &requests_[0]           // array_of_requests
            ,   &idx
            ,   &flag
            ,   &status
            )
        );
        
        // found active handles and found one completions   -> flag = true  , index = valid index
        // found active handles but no completion           -> flag = false , index = MPI_UNDEFINED
        // no active handles                                -> flag = true  , index = MPI_UNDEFINED
        
        if (idx != MPI_UNDEFINED)
        {
            const index_t index = static_cast<index_t>(idx);
            
            MPI_Status* const status_result = statuses_[index];
            if (status_result != MPI_STATUS_IGNORE)
                *statuses_[index] = status;
            
            // Execute the callback.
            completions_[index]();
            
            MGBASE_LOG_DEBUG(
                "msg:Completed MPI request.\t"
                "index:{}\tsource_rank:{}\ttag:{}"
            ,   index
            ,   status.MPI_SOURCE
            ,   status.MPI_TAG
            );
            
            queue_.push_back(index);
            
            --established_;
        }
    }
    
private:
    bool initialized_;
    
    mgbase::circular_buffer<index_t>            queue_;
    
    mgbase::scoped_ptr<MPI_Status* []>          statuses_;
    mgbase::scoped_ptr<MPI_Request []>          requests_;
    
    mgbase::scoped_ptr<mgbase::callback<void ()> []>    completions_;
    
    int established_;
};

} // namespace mpi
} // namespace mgcom

