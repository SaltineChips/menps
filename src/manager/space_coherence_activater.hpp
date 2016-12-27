
#pragma once

#include "mgdsm_common.hpp"

namespace mgdsm {

class space_coherence_activater
{
public:
    typedef mgcom::process_id_t process_id_type;
    typedef segment_id_t        segment_id_type;
    typedef page_id_t           page_id_type;
    
    virtual ~space_coherence_activater() /*noexcept*/ = default;
    
    virtual void enable_flush(process_id_type, segment_id_type, page_id_type) = 0;
    
    virtual void enable_diff(process_id_type, segment_id_type, page_id_type) = 0;
};

} // namespace mgdsm

