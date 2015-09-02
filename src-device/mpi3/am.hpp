
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

namespace mgcom {

namespace am {

void initialize();

void finalize();

inline MPI_Comm& get_comm() {
    static MPI_Comm comm;
    return comm;
}

}

}

