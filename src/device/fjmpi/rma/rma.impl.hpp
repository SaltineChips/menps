
#pragma once

#include "common/rma.hpp"
#include "device/fjmpi/fjmpi.hpp"
#include "device/fjmpi/fjmpi_error.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "common/command/comm_call.hpp"

#include <mgbase/lockfree/mpmc_bounded_queue.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace fjmpi {

namespace /*unnamed*/ {

class rma_impl
{
    static const int max_memid_count = 510;
    static const int max_tag_count = 15;
   
public:
    static const int max_nic_count = 4;
    
    rma_impl()
        : memid_queue_(max_tag_count + 1) { }
    
    void initialize()
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_init()
        );
        
        for (int memid = 0; memid < max_memid_count; memid++)
            memid_queue_.enqueue(memid);
        
        MGBASE_LOG_DEBUG("msg:Initialized FJMPI.");
        
        // This barrier seems necessary
        // to ensure that all processes finished initialization.
        //mpi::native_barrier();
    }
    
    void finalize()
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_finalize()
        );
        
        MGBASE_LOG_DEBUG("msg:Finalized FJMPI.");
    }
    
private:
    struct register_memory_closure
    {
        mgbase::uint64_t operator () () const
        {
            const mgbase::uint64_t laddr = fjmpi_error::assert_not_error(
                FJMPI_Rdma_reg_mem(memid, buf, length)
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Registered region.\tptr:{:x}\tsize:{}\tladdr:{:x}"
            ,   reinterpret_cast<mgbase::uintptr_t>(buf)
            ,   length
            ,   laddr
            );
            
            return laddr;
        }
        
        int                 memid;
        void*               buf;
        std::size_t         length;
    };
    
public:
    int register_memory(void* const buf, const std::size_t length, mgbase::uint64_t* const laddr_result)
    {
        int memid;
        if (!memid_queue_.dequeue(memid)) {
            MGBASE_LOG_WARN("Exceeded the capacity of memid");
            throw fjmpi_error(); // exceeded the capacity of memid
        }
        
        const register_memory_closure cl = { memid, buf, length };
        const mgbase::uint64_t laddr = comm_call<mgbase::uint64_t>(cl);
        
        *laddr_result = laddr;
        
        return memid;
    }
    
private:
    struct get_remote_addr_closure
    {
        mgbase::uint64_t operator () () const
        {
            const mgbase::uint64_t raddr = fjmpi_error::assert_not_error(
                FJMPI_Rdma_get_remote_addr(pid, memid)
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Got remote address.\tproc:{}\tmemid:{}\traddr:{:x}"
            ,   pid
            ,   memid
            ,   raddr
            );
            
            return raddr;
        }
        
        int pid;
        int memid;
    };
    
public:
    mgbase::uint64_t get_remote_addr(const int pid, const int memid)
    {
        const get_remote_addr_closure cl = { pid, memid };
        const mgbase::uint64_t raddr = comm_call<mgbase::uint64_t>(cl);
        return raddr;
    }
    
private:
    struct deregister_memory_closure
    {
        void operator () () const
        {
            fjmpi_error::assert_zero(
                FJMPI_Rdma_dereg_mem(memid)
            );
        }
        
        int memid;
    };
    
public:
    void deregister_memory(const int memid)
    {
        const deregister_memory_closure cl = { memid };
        comm_call<void>(cl);
        
        memid_queue_.enqueue(memid);
        
        MGBASE_LOG_DEBUG(
            "msg:Deregistered region.\tmemid:{}"
        ,   memid
        );
    }
   
private:
    mgbase::mpmc_bounded_queue<int> memid_queue_;
};

} // unnamed namespace

} // namespace fjmpi
} // namespace mgcom

