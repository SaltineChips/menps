
#include <mgdev/mpi/mpi_error.hpp>
#include <mgbase/external/fmt.hpp>

namespace mgdev {
namespace mpi {

std::string get_error_string(const int err_code) MGBASE_NOEXCEPT
{
    char result[MPI_MAX_ERROR_STRING];
    int len;
    
    MPI_Error_string(err_code, result, &len); // Ignore error
    
    return result;
}

int get_error_class(const int err_code) MGBASE_NOEXCEPT
{
    int err_class;
    
    MPI_Error_class(err_code, &err_class); // Ignore error
    
    return err_class;
}

std::string get_error_description(const int err_code) MGBASE_NOEXCEPT
{
    const auto err_class = get_error_class(err_code);
    const auto code_str = get_error_string(err_code);
    const auto class_str = get_error_string(err_class);
    
    return fmt::format(
        "{}:{}, {}:{}"
    ,   err_class
    ,   class_str
    ,   err_code
    ,   code_str
    );
}

std::string get_comm_name(const MPI_Comm comm) MGBASE_NOEXCEPT
{
    char name[MPI_MAX_OBJECT_NAME];
    int len;
    
    mpi_error::check(
        MPI_Comm_get_name(comm, name, &len)
    );
    
    return name;
}

} // namespace mpi
} // namespace mgdev

