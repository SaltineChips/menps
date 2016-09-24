
#pragma once

#include <mgcom/common.hpp>

namespace mgult {

typedef mgbase::uint32_t    ult_local_id_t;

union ult_id
{
    void* ptr;
    
    struct dist
    {
        mgcom::process_id_t proc;
        ult_local_id_t      local_id;
    };
};

MGBASE_STATIC_ASSERT(mgbase::is_trivially_copyable<ult_id>::value);

//MGBASE_CONSTEXPR
inline ult_id make_invalid_ult_id() MGBASE_NOEXCEPT
{
    return { reinterpret_cast<void*>(-1) };
}

//MGBASE_CONSTEXPR
inline bool is_invalid_ult_id(const ult_id& id) MGBASE_NOEXCEPT
{
    return id.ptr == make_invalid_ult_id().ptr;
}

} // namespace mgult

