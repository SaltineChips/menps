
#pragma once

#include <mgbase/memory/next_in_bytes.hpp>

namespace mgdsm {

template <typename Traits>
class basic_sharer_segment_accessor
{
    typedef typename Traits::derived_type               derived_type;
    
    typedef typename Traits::page_id_type               page_id_type;
    
    typedef typename Traits::difference_type            difference_type;
    
    typedef typename Traits::acquire_read_result_type   acquire_read_result_type;
    typedef typename Traits::acquire_write_result_type  acquire_write_result_type;
    
public:
    acquire_read_result_type acquire_read_page(const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        auto& manager = self.get_manager();
        
        return manager.acquire_read(pg_id);
    }
    
    void release_read_page(const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        auto& manager = self.get_manager();
        
        manager.release_read(pg_id);
    }
    
    acquire_write_result_type acquire_write_page(const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        auto& manager = self.get_manager();
        
        return manager.acquire_write(pg_id);
    }
    
    void release_write_page(const page_id_type pg_id)
    {
        auto& self = this->derived();
        
        auto& manager = self.get_manager();
        
        manager.release_write(pg_id);
    }
    
    mgbase::size_t get_page_size() const MGBASE_NOEXCEPT
    {
        auto& self = this->derived();
        auto& seg_ent = self.get_segment_entry();
        
        return seg_ent.get_page_size();
    }
    
    void* get_page_sys_ptr(const page_id_type pg_id)
    {
        auto& self = this->derived();
        auto& seg_ent = self.get_segment_entry();
        
        void* const seg_ptr = seg_ent.get_sys_ptr();
        
        const auto pg_offset = this->get_page_offset(pg_id);
        
        return mgbase::next_in_bytes(seg_ptr, pg_offset);
    }
    
private:
    difference_type get_page_offset(const page_id_type pg_id)
    {
        auto& self = this->derived();
        auto& seg_ent = self.get_segment_entry();
        
        const auto pg_size = seg_ent.get_page_size();
        
        return static_cast<difference_type>(pg_size * pg_id);
    }
    
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    const derived_type& derived() const MGBASE_NOEXCEPT {
        return static_cast<const derived_type&>(*this);
    }
};

} // namespace mgdsm

