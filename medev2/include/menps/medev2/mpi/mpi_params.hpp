
#pragma once

#include <menps/medev2/mpi/mpi.hpp>

namespace menps {
namespace medev2 {
namespace mpi {

struct send_params
{
    const void* buf;
    int         num_bytes;
    int         dest_rank;
    int         tag;
    MPI_Comm    comm;
};

struct recv_params
{
    void*       buf;
    int         num_bytes;
    int         src_rank;
    int         tag;
    MPI_Comm    comm;
    MPI_Status* status_result;
};

struct isend_params
{
    const void*     buf;
    int             count;
    MPI_Datatype    datatype;
    int             dest_rank;
    int             tag;
    MPI_Comm        comm;
    MPI_Request*    request;
};

struct irecv_params
{
    void*           buf;
    int             count;
    MPI_Datatype    datatype;
    int             src_rank;
    int             tag;
    MPI_Comm        comm;
    MPI_Request*    request;
};

struct test_params
{
    MPI_Request*    request;
    int*            flag_result;
    MPI_Status*     status_result;
};

struct probe_params
{
    int         src_rank;
    int         tag;
    MPI_Comm    comm;
    MPI_Status* status_result;
};

struct barrier_params
{
    MPI_Comm    comm;
};

struct broadcast_params
{
    void*       ptr;
    int         num_bytes;
    int         root;
    MPI_Comm    comm;
};

struct allgather_params
{
    const void* src;
    void*       dest;
    int         num_bytes;
    MPI_Comm    comm;
};

struct alltoall_params
{
    const void*     sendbuf;
    int             sendcount;
    MPI_Datatype    sendtype;
    void*           recvbuf;
    int             recvcount;
    MPI_Datatype    recvtype;
    MPI_Comm        comm;
};

struct allreduce_params
{
    const void*     sendbuf;
    void*           recvbuf;
    int             count;
    MPI_Datatype    datatype;
    MPI_Op          op;
    MPI_Comm        comm;
};

struct get_params
{
    void*       dest_ptr;
    int         src_rank;
    MPI_Aint    src_index;
    int         num_bytes;
    MPI_Win     win;
};

struct put_params
{
    const void* src_ptr;
    int         dest_rank;
    MPI_Aint    dest_index;
    int         num_bytes;
    MPI_Win     win;
};

struct compare_and_swap_params
{
    const void*     desired_ptr;
    const void*     expected_ptr;
    void*           result_ptr;
    MPI_Datatype    datatype;
    int             target_rank;
    MPI_Aint        target_index;
    MPI_Win         win;
};

#if 0
struct fetch_and_op_params
{
    mefdn::uint64_t    value;
    void*               result_ptr;
    MPI_Datatype        datatype;
    int                 dest_rank;
    MPI_Aint            dest_index;
    MPI_Op              operation;
    MPI_Win             win;
};
#endif

struct rput_params
{
    const void*     origin_addr;
    int             origin_count;
    MPI_Datatype    origin_datatype;
    int             target_rank;
    MPI_Aint        target_disp;
    int             target_count;
    MPI_Datatype    target_datatype;
    MPI_Win         win;
    MPI_Request*    request;
};

struct rget_params
{
    void*           origin_addr;
    int             origin_count;
    MPI_Datatype    origin_datatype;
    int             target_rank;
    MPI_Aint        target_disp;
    int             target_count;
    MPI_Datatype    target_datatype;
    MPI_Win         win;
    MPI_Request*    request;
};

struct rget_accumulate_params
{
    const void*     origin_addr;
    int             origin_count;
    MPI_Datatype    origin_datatype;
    void*           result_addr;
    int             result_count;
    MPI_Datatype    result_datatype;
    int             target_rank;
    MPI_Aint        target_disp;
    int             target_count;
    MPI_Datatype    target_datatype;
    MPI_Op          op;
    MPI_Win         win;
    MPI_Request*    request;
};

struct win_flush_all_params
{
    MPI_Win     win;
};

struct win_flush_params
{
    int         rank;
    MPI_Win     win;
};

struct win_flush_local_params
{
    int         rank;
    MPI_Win     win;
};

struct win_create_dynamic_params
{
    MPI_Info    info;
    MPI_Comm    comm;
};

struct win_attach_params
{
    MPI_Win     win;
    void*       base;
    MPI_Aint    size;
};

struct win_detach_params
{
    MPI_Win     win;
    void*       base;
};

struct win_lock_all_params
{
    int         assert;
    MPI_Win     win;
};
struct win_unlock_all_params
{
    MPI_Win     win;
};

} // namespace mpi
} // namespace medev2
} // namespace menps

