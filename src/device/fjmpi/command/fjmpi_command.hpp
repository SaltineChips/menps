
#pragma once

#include "fjmpi_completer.hpp"
#include "device/mpi/command/mpi_command.hpp"

namespace mgcom {
namespace fjmpi {

enum fjmpi_command_code
{
    FJMPI_COMMAND_GET = mpi::MPI_COMMAND_END + 1
,   FJMPI_COMMAND_PUT
,   FJMPI_COMMAND_END
};

union fjmpi_command_parameters
{
    mpi::mpi_command_parameters mpi1;
    
    struct contiguous_parameters {
        int                 proc;
        mgbase::uint64_t    laddr;
        mgbase::uint64_t    raddr;
        std::size_t         size_in_bytes;
        int                 nic;
        int                 extra_flags;
        mgbase::operation   on_complete;
    }
    contiguous;
};

MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const fjmpi_command_code        code
,   const fjmpi_command_parameters& params
,   fjmpi_completer&                completer
) {
    if (code < static_cast<fjmpi_command_code>(mpi::MPI_COMMAND_END)) {
        return mpi::execute_on_this_thread(
            static_cast<mpi::mpi_command_code>(code)
        ,   params.mpi1
        ,   completer.get_mpi1_completer()
        );
    }
    
    switch (code)
    {
        case FJMPI_COMMAND_GET: {
            const fjmpi_command_parameters::contiguous_parameters& p = params.contiguous;
            
            int tag;
            const bool found_tag = completer.try_new_tag(p.proc, p.nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
            {
                const int flags = p.nic | p.extra_flags;
                
                fjmpi_error::assert_zero(
                    FJMPI_Rdma_get(p.proc, tag, p.raddr, p.laddr, p.size_in_bytes, flags)
                );
                
                completer.set_notification(p.proc, p.nic, tag, p.on_complete);
            }
            
            MGBASE_LOG_DEBUG(
                "msg:{}\t"
                "src_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tnic:{}\textra_flags:{}"
            ,   (found_tag ? "Executed FJMPI_Rdma_get." : "RDMA Get because tag capacity exceeded.")
            ,   p.proc
            ,   p.laddr
            ,   p.raddr
            ,   p.size_in_bytes
            ,   p.nic
            ,   p.extra_flags
            );
            
            return found_tag;
        }
        
        case FJMPI_COMMAND_PUT: {
            const fjmpi_command_parameters::contiguous_parameters& p = params.contiguous;
            
            int tag;
            const bool found_tag = completer.try_new_tag(p.proc, p.nic, &tag);
            
            if (MGBASE_LIKELY(found_tag))
            {
                const int flags = p.nic | p.extra_flags;
                
                fjmpi_error::assert_zero(
                    FJMPI_Rdma_put(p.proc, tag, p.raddr, p.laddr, p.size_in_bytes, flags)
                );
                
                completer.set_notification(p.proc, p.nic, tag, p.on_complete);
            }
            
            MGBASE_LOG_DEBUG(
                "msg:{}\t"
                "dest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tnic:{}\tflags:{}"
            ,   (found_tag ? "Executed FJMPI_Rdma_put." : "RDMA Put failed because tag capacity exceeded.")
            ,   p.proc
            ,   p.laddr
            ,   p.raddr
            ,   p.size_in_bytes
            ,   p.nic
            ,   p.extra_flags
            );
            
            return found_tag;
        }
        
        default:
            MGBASE_UNREACHABLE();
    }
}

} // namespace fjmpi
} // namespace mgcom

