
#pragma once

#include "sharer/sharer_space.hpp"
#include "app_space.hpp"

namespace mgdsm {

class access_history
{
    typedef mgbase::mutex   mutex_type;
    
public:
    struct conf {
        sharer_space& sp;
        app_space& app_sp;
    };
    
    explicit access_history(const conf& cnf)
        : conf_(cnf)
    { }
    
    struct abs_block_id
    {
        segment_id_t    seg_id;
        page_id_t       pg_id;
        block_id_t      blk_id;
    };
    
    void add_new_read(const abs_block_id ablk_id)
    {
        mgbase::lock_guard<mutex_type> lk(this->read_mtx_);
        
        this->read_ids_.push_back(ablk_id);
    }
    
    void add_new_write(const abs_block_id ablk_id)
    {
        mgbase::lock_guard<mutex_type> lk(this->write_mtx_);
        
        this->write_ids_.push_back(ablk_id);
    }
    
    void read_barrier()
    {
        mgbase::lock_guard<mutex_type> lk(this->read_mtx_);
        
        for (const auto& id : read_ids_)
        {
            auto seg_ac = this->conf_.sp.get_segment_accessor(id.seg_id);
            
            auto pg_ac = seg_ac.get_page_accessor(id.pg_id);
            
            auto blk_ac = pg_ac.get_block_accessor(id.blk_id);
            
            blk_ac.flush();
        }
        
        this->read_ids_.clear();
    }
    
    void write_barrier()
    {
        mgbase::lock_guard<mutex_type> lk(this->write_mtx_);
        
        for (const auto& id : this->write_ids_)
        {
            auto seg_ac = this->conf_.sp.get_segment_accessor(id.seg_id);
            
            auto pg_ac = seg_ac.get_page_accessor(id.pg_id);
            
            auto blk_ac = pg_ac.get_block_accessor(id.blk_id);
            
            blk_ac.reconcile();
        }
        
        this->write_ids_.clear();
    }
    
private:
    const conf conf_;
    
    mutex_type read_mtx_;
    std::vector<abs_block_id> read_ids_;
    
    mutex_type write_mtx_;
    std::vector<abs_block_id> write_ids_;
};

} // namespace mgdsm

